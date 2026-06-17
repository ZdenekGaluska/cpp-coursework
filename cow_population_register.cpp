#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

// Population register with copy-on-write (COW) semantics.
//
// Internally maintains a sorted array of citizens (by national ID) and, per citizen,
// a sorted array of residency records (by date). Copying the register is O(1)
// until either copy is mutated, at which point a full deep copy is made.
class PopulationRegister {
    // Result of a binary search: index is the insertion point (or found position),
    // found indicates whether an exact match exists at that position.
    struct SearchResult {
        bool   found;
        size_t index;
    };

    struct ResidencyRecord {
        char  date[12];
        char* street;
        char* city;
    };

    struct Citizen {
        char  id[12];
        char* name;
        char* surname;

        ResidencyRecord* history;
        size_t           count;
        size_t           capacity;
    };

    // Reference-counted shared data block — the backbone of COW.
    struct SharedData {
        Citizen* citizens;
        size_t   count;
        size_t   capacity;
        size_t   refCount;

        SharedData() : citizens(nullptr), count(0), capacity(0), refCount(1) {}

        ~SharedData() {
            for (size_t i = 0; i < count; ++i) {
                for (size_t j = 0; j < citizens[i].count; ++j) {
                    delete[] citizens[i].history[j].city;
                    delete[] citizens[i].history[j].street;
                }
                delete[] citizens[i].name;
                delete[] citizens[i].surname;
                delete[] citizens[i].history;
            }
            delete[] citizens;
        }
    };

    SharedData* shared;

    // Grows the citizens array if at capacity.
    void reallocateIfNeeded() {
        if (shared->count < shared->capacity) return;
        if (shared->capacity != 0) {
            shared->capacity += 2;
            shared->capacity *= 2;
            Citizen* newBuffer = new Citizen[shared->capacity];
            for (size_t i = 0; i < shared->count; ++i)
                newBuffer[i] = shared->citizens[i];
            delete[] shared->citizens;
            shared->citizens = newBuffer;
        } else {
            shared->capacity = 8;
            shared->citizens = new Citizen[shared->capacity];
        }
    }

    // Binary search for a citizen by national ID.
    SearchResult findPosition(const char id[]) const {
        SearchResult result = {false, 0};
        size_t low = 0, high = shared->count;
        while (low < high) {
            size_t mid = (low + high) / 2;
            int cmp = std::strcmp(shared->citizens[mid].id, id);
            if (cmp < 0) low  = mid + 1;
            else         high = mid;
        }
        if (low < shared->count && std::strcmp(shared->citizens[low].id, id) == 0)
            result.found = true;
        result.index = low;
        return result;
    }

    // Binary search for a date within a citizen's residency history.
    SearchResult findPosition(const char date[], size_t citizenIdx) const {
        SearchResult result = {false, 0};
        size_t low = 0, high = shared->citizens[citizenIdx].count;
        while (low < high) {
            size_t mid = (low + high) / 2;
            int cmp = std::strcmp(shared->citizens[citizenIdx].history[mid].date, date);
            if (cmp < 0) low  = mid + 1;
            else         high = mid;
        }
        if (low < shared->citizens[citizenIdx].count &&
            std::strcmp(shared->citizens[citizenIdx].history[low].date, date) == 0)
            result.found = true;
        result.index = low;
        return result;
    }

    // Inserts a residency record at position pos in a citizen's history array.
    void insertResidency(size_t citizenIdx, size_t pos, const ResidencyRecord& rec) {
        Citizen& c = shared->citizens[citizenIdx];
        for (size_t i = c.count; i > pos; --i)
            c.history[i] = c.history[i - 1];
        c.history[pos] = rec;
        ++c.count;
    }

    // Inserts a citizen at position pos in the main citizens array.
    void insertCitizen(int pos, const Citizen& c) {
        for (int i = static_cast<int>(shared->count); i > pos; --i)
            shared->citizens[i] = shared->citizens[i - 1];
        shared->citizens[pos] = c;
        ++shared->count;
    }

    // COW trigger: detaches from the shared block and makes a private deep copy.
    void ensureUnique() {
        if (shared->refCount <= 1) return;

        SharedData* copy     = new SharedData;
        copy->capacity       = shared->capacity;
        copy->count          = shared->count;
        copy->citizens       = new Citizen[copy->capacity];

        for (size_t i = 0; i < shared->count; ++i) {
            std::strcpy(copy->citizens[i].id, shared->citizens[i].id);

            copy->citizens[i].name    = new char[std::strlen(shared->citizens[i].name)    + 1];
            copy->citizens[i].surname = new char[std::strlen(shared->citizens[i].surname) + 1];
            std::strcpy(copy->citizens[i].name,    shared->citizens[i].name);
            std::strcpy(copy->citizens[i].surname, shared->citizens[i].surname);

            copy->citizens[i].capacity = shared->citizens[i].capacity;
            copy->citizens[i].count    = shared->citizens[i].count;
            copy->citizens[i].history  = new ResidencyRecord[copy->citizens[i].capacity];

            for (size_t j = 0; j < shared->citizens[i].count; ++j) {
                std::strcpy(copy->citizens[i].history[j].date, shared->citizens[i].history[j].date);
                copy->citizens[i].history[j].street = new char[std::strlen(shared->citizens[i].history[j].street) + 1];
                copy->citizens[i].history[j].city   = new char[std::strlen(shared->citizens[i].history[j].city)   + 1];
                std::strcpy(copy->citizens[i].history[j].street, shared->citizens[i].history[j].street);
                std::strcpy(copy->citizens[i].history[j].city,   shared->citizens[i].history[j].city);
            }
        }

        copy->refCount = 1;
        --shared->refCount;
        shared = copy;
    }

public:
    PopulationRegister() : shared(new SharedData) {}

    ~PopulationRegister() {
        if (--shared->refCount == 0) delete shared;
    }

    // Shallow copy — both instances share data until one mutates (COW).
    PopulationRegister(const PopulationRegister& other) : shared(other.shared) {
        ++shared->refCount;
    }

    PopulationRegister& operator=(const PopulationRegister& other) {
        if (this != &other) {
            ++other.shared->refCount;
            if (--shared->refCount == 0) delete shared;
            shared = other.shared;
        }
        return *this;
    }

    // Registers a new citizen with an initial residency record.
    // Returns false if a citizen with this ID already exists.
    bool add(const char id[], const char name[], const char surname[],
             const char date[], const char street[], const char city[]) {
        ensureUnique();
        reallocateIfNeeded();

        SearchResult pos = findPosition(id);
        if (pos.found) return false;

        Citizen c;
        std::strcpy(c.id, id);
        c.name    = new char[std::strlen(name)    + 1];
        c.surname = new char[std::strlen(surname) + 1];
        std::strcpy(c.name,    name);
        std::strcpy(c.surname, surname);

        c.history           = new ResidencyRecord[1];
        c.history[0].city   = new char[std::strlen(city)   + 1];
        c.history[0].street = new char[std::strlen(street) + 1];
        std::strcpy(c.history[0].city,   city);
        std::strcpy(c.history[0].street, street);
        std::strcpy(c.history[0].date,   date);
        c.count    = 1;
        c.capacity = 1;

        insertCitizen(static_cast<int>(pos.index), c);
        return true;
    }

    // Adds a new residency record for an existing citizen.
    // Returns false if the citizen is not found or this date is already recorded.
    bool resettle(const char id[], const char date[], const char street[], const char city[]) {
        ensureUnique();

        SearchResult pos = findPosition(id);
        if (!pos.found) return false;

        Citizen& c = shared->citizens[pos.index];
        if (c.count >= c.capacity) {
            c.capacity = c.capacity * 2 + 2;
            ResidencyRecord* newHistory = new ResidencyRecord[c.capacity];
            for (size_t i = 0; i < c.count; ++i)
                newHistory[i] = c.history[i];
            delete[] c.history;
            c.history = newHistory;
        }

        SearchResult datePos = findPosition(date, pos.index);
        if (datePos.found) return false;

        ResidencyRecord entry;
        entry.city   = new char[std::strlen(city)   + 1];
        entry.street = new char[std::strlen(street) + 1];
        std::strcpy(entry.city,   city);
        std::strcpy(entry.street, street);
        std::strcpy(entry.date,   date);

        insertResidency(pos.index, datePos.index, entry);
        return true;
    }

    // Writes a citizen's full record (ID, name, surname, residency history) to os.
    // Returns false if the citizen is not found.
    bool print(std::ostream& os, const char id[]) const {
        SearchResult pos = findPosition(id);
        if (!pos.found) return false;

        const Citizen& c = shared->citizens[pos.index];
        os << c.id << " " << c.name << " " << c.surname << "\n";
        for (size_t i = 0; i < c.count; ++i)
            os << c.history[i].date << " " << c.history[i].street << " " << c.history[i].city << "\n";
        return true;
    }
};


// --- Tests ---

int main() {
    char lID[12], lDate[12], lName[50], lSurname[50], lStreet[50], lCity[50];
    std::ostringstream oss;

    PopulationRegister a;
    assert(a.add("123456/7890", "John",   "Smith",  "2000-01-01", "Main street",     "Seattle")    == true);
    assert(a.add("987654/3210", "Freddy", "Kruger", "2001-02-03", "Elm street",      "Sacramento") == true);
    assert(a.resettle("123456/7890", "2003-05-12", "Elm street",       "Atlanta")     == true);
    assert(a.resettle("123456/7890", "2002-12-05", "Sunset boulevard", "Los Angeles") == true);

    oss.str("");
    assert(a.print(oss, "123456/7890") == true);
    assert(!std::strcmp(oss.str().c_str(), R"###(123456/7890 John Smith
2000-01-01 Main street Seattle
2002-12-05 Sunset boulevard Los Angeles
2003-05-12 Elm street Atlanta
)###"));

    oss.str("");
    assert(a.print(oss, "987654/3210") == true);
    assert(!std::strcmp(oss.str().c_str(), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
)###"));

    // Verify COW: modifying b does not affect a and vice versa.
    PopulationRegister b(a);
    assert(b.resettle("987654/3210", "2008-04-12", "Elm street", "Cinccinati")   == true);
    assert(a.resettle("987654/3210", "2007-02-11", "Elm street", "Indianapolis") == true);

    oss.str("");
    assert(a.print(oss, "987654/3210") == true);
    assert(!std::strcmp(oss.str().c_str(), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2007-02-11 Elm street Indianapolis
)###"));

    oss.str("");
    assert(b.print(oss, "987654/3210") == true);
    assert(!std::strcmp(oss.str().c_str(), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
)###"));

    a = b;
    assert(a.resettle("987654/3210", "2011-05-05", "Elm street", "Salt Lake City") == true);

    oss.str("");
    assert(a.print(oss, "987654/3210") == true);
    assert(!std::strcmp(oss.str().c_str(), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
2011-05-05 Elm street Salt Lake City
)###"));

    oss.str("");
    assert(b.print(oss, "987654/3210") == true);
    assert(!std::strcmp(oss.str().c_str(), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
)###"));

    assert(b.add("987654/3210", "Joe", "Lee", "2010-03-17", "Abbey road", "London") == false);
    assert(a.resettle("987654/3210", "2001-02-03", "Second street", "Milwaukee")    == false);

    oss.str("");
    assert(a.print(oss, "666666/6666") == false);

    // Verify no dependency on string literal lifetimes (stack-allocated buffers).
    PopulationRegister c;
    std::strncpy(lID,      "123456/7890", sizeof(lID));
    std::strncpy(lName,    "John",        sizeof(lName));
    std::strncpy(lSurname, "Smith",       sizeof(lSurname));
    std::strncpy(lDate,    "2000-01-01",  sizeof(lDate));
    std::strncpy(lStreet,  "Main street", sizeof(lStreet));
    std::strncpy(lCity,    "Seattle",     sizeof(lCity));
    assert(c.add(lID, lName, lSurname, lDate, lStreet, lCity) == true);

    std::strncpy(lID,      "987654/3210", sizeof(lID));
    std::strncpy(lName,    "Freddy",      sizeof(lName));
    std::strncpy(lSurname, "Kruger",      sizeof(lSurname));
    std::strncpy(lDate,    "2001-02-03",  sizeof(lDate));
    std::strncpy(lStreet,  "Elm street",  sizeof(lStreet));
    std::strncpy(lCity,    "Sacramento",  sizeof(lCity));
    assert(c.add(lID, lName, lSurname, lDate, lStreet, lCity) == true);

    std::strncpy(lID,     "123456/7890", sizeof(lID));
    std::strncpy(lDate,   "2003-05-12",  sizeof(lDate));
    std::strncpy(lStreet, "Elm street",  sizeof(lStreet));
    std::strncpy(lCity,   "Atlanta",     sizeof(lCity));
    assert(c.resettle(lID, lDate, lStreet, lCity) == true);

    std::strncpy(lID,     "123456/7890",      sizeof(lID));
    std::strncpy(lDate,   "2002-12-05",        sizeof(lDate));
    std::strncpy(lStreet, "Sunset boulevard",  sizeof(lStreet));
    std::strncpy(lCity,   "Los Angeles",        sizeof(lCity));
    assert(c.resettle(lID, lDate, lStreet, lCity) == true);

    oss.str("");
    assert(c.print(oss, "123456/7890") == true);
    assert(!std::strcmp(oss.str().c_str(), R"###(123456/7890 John Smith
2000-01-01 Main street Seattle
2002-12-05 Sunset boulevard Los Angeles
2003-05-12 Elm street Atlanta
)###"));

    std::cout << "All tests passed." << std::endl;
    return EXIT_SUCCESS;
}
