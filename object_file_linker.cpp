#include <cstring>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <set>
#include <map>
#include <queue>

// Holds all data parsed from a single .o object file.
struct ObjectFile {
    std::string fileName;
    std::map<std::string, uint32_t>              exports;  // symbol → byte offset in body
    std::map<std::string, std::vector<uint32_t>> imports;  // symbol → list of use-site offsets
    std::vector<uint8_t>                         compiledCode;
    uint32_t                                     bodySize;
};

// Minimal static linker: loads multiple object files, resolves symbols,
// and produces a single flat binary starting from a specified entry point.
class Linker {
    // Per-function metadata collected across all loaded object files.
    struct FunctionInfo {
        std::string name;
        std::string sourceFile;  // which ObjectFile owns this function
        uint32_t    orgOffset;   // original byte offset within the source body
        uint32_t    newOffset;   // assigned byte offset in the output binary
        uint32_t    size;        // byte count of the function body
    };

    // Parses a binary .o file into an ObjectFile struct.
    //
    // Binary layout:
    //   header  : [exportCount : u32][importCount : u32][bodySize : u32]
    //   exports : ([nameLen : u8][name : nameLen bytes][offset : u32]) × exportCount
    //   imports : ([nameLen : u8][name : nameLen bytes]
    //               [useCount : u32][offset : u32] × useCount) × importCount
    //   body    : raw bytes × bodySize
    bool loadObjectFile(const std::string& path, ObjectFile& obj) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return false;

        uint32_t exportCount, importCount, bodySize;
        file.read(reinterpret_cast<char*>(&exportCount), sizeof(exportCount));
        file.read(reinterpret_cast<char*>(&importCount), sizeof(importCount));
        file.read(reinterpret_cast<char*>(&bodySize),    sizeof(bodySize));
        obj.bodySize = bodySize;

        for (uint32_t i = 0; i < exportCount; ++i) {
            uint8_t nameLen;
            file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            std::string name(nameLen, '\0');
            file.read(name.data(), nameLen);
            uint32_t offset;
            file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
            if (!file) return false;

            if (obj.exports.count(name))
                throw std::runtime_error("Duplicate symbol within file: " + name);
            obj.exports[name] = offset;
        }

        for (uint32_t i = 0; i < importCount; ++i) {
            uint8_t nameLen;
            file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            std::string name(nameLen, '\0');
            file.read(name.data(), nameLen);
            uint32_t useCount;
            file.read(reinterpret_cast<char*>(&useCount), sizeof(useCount));
            if (!file) return false;

            for (uint32_t j = 0; j < useCount; ++j) {
                uint32_t offset;
                file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
                if (!file) return false;
                obj.imports[name].push_back(offset);
            }
        }

        obj.compiledCode.resize(bodySize);
        file.read(reinterpret_cast<char*>(obj.compiledCode.data()), bodySize);
        return true;
    }

    // Collects all exported symbols across all loaded files into a flat map.
    // Throws on duplicate symbols across files.
    void buildExportMap(std::map<std::string, FunctionInfo>& exportMap) {
        for (const auto& file : files) {
            for (const auto& [name, offset] : file.exports) {
                if (exportMap.count(name))
                    throw std::runtime_error("Duplicate symbol: " + name);

                // Function size = distance to the next export's offset (or end of body).
                uint32_t endOffset = file.bodySize;
                for (const auto& [otherName, otherOffset] : file.exports) {
                    if (otherOffset > offset && otherOffset < endOffset)
                        endOffset = otherOffset;
                }

                exportMap[name] = { name, file.fileName, offset, 0, endOffset - offset };
            }
        }
    }

    // BFS from entryPoint over the import graph to find all transitively needed functions.
    // Throws if any required symbol has no definition.
    void findReachableFunctions(
        std::set<std::string>&               reachable,
        std::map<std::string, FunctionInfo>& exportMap,
        const std::string&                   entryPoint)
    {
        std::queue<std::string> queue;
        queue.push(entryPoint);
        reachable.insert(entryPoint);

        while (!queue.empty()) {
            const std::string current = queue.front();
            queue.pop();

            if (!exportMap.count(current))
                throw std::runtime_error("Undefined symbol: " + current);

            const FunctionInfo& info = exportMap.at(current);

            for (const auto& file : files) {
                if (file.fileName != info.sourceFile) continue;

                const uint32_t funcStart = info.orgOffset;
                const uint32_t funcEnd   = funcStart + info.size;

                for (const auto& [importName, importOffsets] : file.imports) {
                    for (uint32_t useOffset : importOffsets) {
                        if (useOffset < funcStart || useOffset >= funcEnd) continue;

                        if (!exportMap.count(importName))
                            throw std::runtime_error("Undefined symbol: " + importName);

                        if (!reachable.count(importName)) {
                            reachable.insert(importName);
                            queue.push(importName);
                        }
                    }
                }
                break;
            }
        }
    }

    // Copies function bodies into a flat output buffer and patches all import
    // call-site addresses to their new positions in the output binary.
    void writeOutputFile(
        const std::string&                         outputPath,
        uint32_t                                   totalSize,
        const std::vector<FunctionInfo*>&          ordered,
        const std::map<std::string, FunctionInfo>& exportMap)
    {
        std::vector<uint8_t> output(totalSize);

        for (const FunctionInfo* func : ordered) {
            for (const auto& file : files) {
                if (file.fileName != func->sourceFile) continue;

                std::memcpy(
                    output.data() + func->newOffset,
                    file.compiledCode.data() + func->orgOffset,
                    func->size);

                // Patch each import reference that falls within this function's body range.
                for (const auto& [importName, importOffsets] : file.imports) {
                    for (uint32_t useOffset : importOffsets) {
                        if (useOffset < func->orgOffset || useOffset >= func->orgOffset + func->size)
                            continue;
                        uint32_t newAddr    = exportMap.at(importName).newOffset;
                        uint32_t outputSlot = func->newOffset + (useOffset - func->orgOffset);
                        std::memcpy(output.data() + outputSlot, &newAddr, sizeof(uint32_t));
                    }
                }
                break;
            }
        }

        std::ofstream out(outputPath, std::ios::binary);
        if (!out) throw std::runtime_error("Cannot create output file: " + outputPath);
        out.write(reinterpret_cast<const char*>(output.data()), output.size());
        if (!out) throw std::runtime_error("Failed to write output file: " + outputPath);
    }

public:
    Linker()  = default;
    ~Linker() = default;
    Linker(const Linker&)            = delete;
    Linker& operator=(const Linker&) = delete;

    // Loads and parses an object file. Returns *this for chaining.
    Linker& addFile(const std::string& path) {
        ObjectFile obj;
        obj.fileName = path;
        if (!loadObjectFile(path, obj))
            throw std::runtime_error("Failed to load object file: " + path);
        files.push_back(std::move(obj));
        return *this;
    }

    // Links all loaded object files into a flat output binary.
    // The entry point function is placed first at offset 0; all reachable
    // functions follow in the order determined by BFS traversal.
    void linkOutput(const std::string& outputPath, const std::string& entryPoint) {
        if (files.empty())
            throw std::runtime_error("No object files loaded");

        std::map<std::string, FunctionInfo> exportMap;
        buildExportMap(exportMap);

        if (!exportMap.count(entryPoint))
            throw std::runtime_error("Undefined entry point: " + entryPoint);

        std::set<std::string> reachable;
        findReachableFunctions(reachable, exportMap, entryPoint);

        // Assign output offsets: entry point first, remaining functions after.
        std::vector<FunctionInfo*> ordered;
        uint32_t offset = 0;

        auto* entry = &exportMap.at(entryPoint);
        entry->newOffset = offset;
        offset += entry->size;
        ordered.push_back(entry);

        for (const auto& name : reachable) {
            if (name == entryPoint) continue;
            auto* func = &exportMap.at(name);
            func->newOffset = offset;
            offset += func->size;
            ordered.push_back(func);
        }

        writeOutputFile(outputPath, offset, ordered, exportMap);
    }

private:
    std::vector<ObjectFile> files;
};


// Note: the tests below require binary .o files from the original FIT CTU assignment
// and will not run without them.
int main() {
    Linker().addFile("0in0.o").linkOutput("0out", "strlen");
    Linker().addFile("1in0.o").linkOutput("1out", "main");
    Linker().addFile("2in0.o").addFile("2in1.o").linkOutput("2out", "main");
    Linker().addFile("3in0.o").addFile("3in1.o").linkOutput("3out", "towersOfHanoi");

    try {
        Linker().addFile("4in0.o").addFile("4in1.o").linkOutput("4out", "unusedFunc");
        assert(false && "expected exception: undefined symbol");
    } catch (const std::runtime_error&) {}

    try {
        Linker().addFile("5in0.o").linkOutput("5out", "main");
        assert(false && "expected exception: duplicate symbol");
    } catch (const std::runtime_error&) {}

    try {
        Linker().addFile("6in0.o").linkOutput("6out", "strlen");
        assert(false && "expected exception: file not found");
    } catch (const std::runtime_error&) {}

    try {
        Linker().addFile("7in0.o").linkOutput("7out", "strlen2");
        assert(false && "expected exception: undefined entry point");
    } catch (const std::runtime_error&) {}

    return EXIT_SUCCESS;
}
