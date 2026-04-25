
#include &lt;iostream&gt;
#include &lt;string&gt;
#include &lt;vector&gt;
#include &lt;map&gt;
#include &lt;set&gt;
#include &lt;fstream&gt;
#include &lt;sstream&gt;
#include &lt;algorithm&gt;
#include &lt;cctype&gt;

class BasicException {
protected:
    const char *message;

public:
    explicit BasicException(const char *_message) {
        message = _message;
    }

    virtual const char *what() const {
        return message;
    }
};

class ArgumentException: public BasicException {
public:
    explicit ArgumentException(const char *_message) : BasicException(_message) {}
};

class IteratorException: public BasicException {
public:
    explicit IteratorException(const char *_message) : BasicException(_message) {}
};

struct Pokemon {
    char name[12];
    int id;
    std::vector&lt;std::string&gt; types;

    Pokemon() : id(0) {
        name[0] = '\0';
    }

    Pokemon(const char *_name, int _id, const std::vector&lt;std::string&gt; &_types) : id(_id) {
        strncpy(name, _name, 11);
        name[11] = '\0';
        types = _types;
    }
};

// Damage multiplier table
const std::map&lt;std::string, std::map&lt;std::string, float&gt;&gt; DAMAGE_MULTIPLIER = {
    {"normal", {{"rock", 0.5f}, {"ghost", 0.0f}}},
    {"fire", {{"fire", 0.5f}, {"water", 0.5f}, {"grass", 2.0f}, {"ice", 2.0f}, {"bug", 2.0f}, {"rock", 0.5f}, {"dragon", 0.5f}}},
    {"water", {{"fire", 2.0f}, {"water", 0.5f}, {"grass", 0.5f}, {"ground", 2.0f}, {"rock", 2.0f}, {"dragon", 0.5f}}},
    {"electric", {{"water", 2.0f}, {"electric", 0.5f}, {"grass", 0.5f}, {"ground", 0.0f}, {"flying", 2.0f}, {"dragon", 0.5f}}},
    {"grass", {{"fire", 0.5f}, {"water", 2.0f}, {"grass", 0.5f}, {"poison", 0.5f}, {"ground", 2.0f}, {"flying", 0.5f}, {"bug", 0.5f}, {"rock", 2.0f}, {"dragon", 0.5f}}},
    {"ice", {{"water", 0.5f}, {"grass", 2.0f}, {"ice", 0.5f}, {"ground", 2.0f}, {"flying", 2.0f}, {"dragon", 2.0f}}},
    {"ground", {{"fire", 2.0f}, {"electric", 2.0f}, {"grass", 0.5f}, {"poison", 2.0f}, {"flying", 0.0f}, {"bug", 0.5f}, {"rock", 2.0f}}}
};

class Pokedex {
private:
    std::map&lt;int, Pokemon&gt; pokemons;
    std::string fileName;
    std::set&lt;std::string&gt; validTypes = {"normal", "fire", "water", "electric", "grass", "ice", "ground"};

    bool isValidName(const char *name) const {
        if (name == nullptr || strlen(name) == 0 || strlen(name) &gt; 10) {
            return false;
        }
        for (int i = 0; name[i] != '\0'; i++) {
            if (!isalpha(name[i])) {
                return false;
            }
        }
        return true;
    }

    bool isValidType(const std::string &amp;type) const {
        return validTypes.find(type) != validTypes.end();
    }

    std::vector&lt;std::string&gt; parseTypes(const char *typesStr) const {
        std::vector&lt;std::string&gt; types;
        if (typesStr == nullptr) return types;

        std::string typesString(typesStr);
        std::stringstream ss(typesString);
        std::string type;
        
        while (std::getline(ss, type, '#')) {
            if (!isValidType(type)) {
                throw ArgumentException(("Argument Error: PM Type Invalid (" + type + ")").c_str());
            }
            types.push_back(type);
        }
        
        return types;
    }

    void loadFromFile() {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            // File doesn't exist, will be created on save
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::stringstream ss(line);
            std::string name;
            int id;
            std::string typesStr;
            
            if (std::getline(ss, name, ',') && ss &gt;&gt; id &gt;&gt; typesStr) {
                // Remove leading space from typesStr if present
                if (!typesStr.empty() &amp;&amp; typesStr[0] == ' ') {
                    typesStr = typesStr.substr(1);
                }
                
                try {
                    std::vector&lt;std::string&gt; types = parseTypes(typesStr.c_str());
                    Pokemon pokemon(name.c_str(), id, types);
                    pokemons[id] = pokemon;
                } catch (const ArgumentException&amp;) {
                    // Skip invalid entries
                    continue;
                }
            }
        }
        file.close();
    }

    void saveToFile() const {
        std::ofstream file(fileName);
        if (!file.is_open()) {
            return;
        }

        for (const auto&amp; pair : pokemons) {
            const Pokemon&amp; pokemon = pair.second;
            file &lt;&lt; pokemon.name &lt;&lt; "," &lt;&lt; pokemon.id;
            
            if (!pokemon.types.empty()) {
                file &lt;&lt; " ";
                for (size_t i = 0; i &lt; pokemon.types.size(); i++) {
                    if (i &gt; 0) file &lt;&lt; "#";
                    file &lt;&lt; pokemon.types[i];
                }
            }
            file &lt;&lt; std::endl;
        }
        file.close();
    }

public:
    explicit Pokedex(const char *_fileName) {
        fileName = _fileName;
        loadFromFile();
    }

    ~Pokedex() {
        saveToFile();
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        // Validate name
        if (!isValidName(name)) {
            throw ArgumentException(("Argument Error: PM Name Invalid (" + std::string(name ? name : "") + ")").c_str());
        }

        // Validate types
        std::vector&lt;std::string&gt; typesVec;
        try {
            typesVec = parseTypes(types);
        } catch (const ArgumentException&amp; e) {
            throw;
        }

        // Check if ID already exists
        if (pokemons.find(id) != pokemons.end()) {
            return false;
        }

        // Add the Pokemon
        Pokemon pokemon(name, id, typesVec);
        pokemons[id] = pokemon;
        return true;
    }

    bool pokeDel(int id) {
        auto it = pokemons.find(id);
        if (it == pokemons.end()) {
            return false;
        }
        pokemons.erase(it);
        return true;
    }

    std::string pokeFind(int id) const {
        auto it = pokemons.find(id);
        if (it == pokemons.end()) {
            return "None";
        }
        return std::string(it-&gt;second.name);
    }

    std::string typeFind(const char *types) const {
        if (types == nullptr || strlen(types) == 0) {
            return "None";
        }

        // Parse the query types
        std::vector&lt;std::string&gt; queryTypes;
        std::stringstream ss(types);
        std::string type;
        while (std::getline(ss, type, '#')) {
            if (!isValidType(type)) {
                throw ArgumentException(("Argument Error: PM Type Invalid (" + type + ")").c_str());
            }
            queryTypes.push_back(type);
        }

        // Find matching Pokemon
        std::vector&lt;const Pokemon*&gt; matches;
        for (const auto&amp; pair : pokemons) {
            const Pokemon&amp; pokemon = pair.second;
            bool match = false;
            for (const auto&amp; queryType : queryTypes) {
                for (const auto&amp; pokemonType : pokemon.types) {
                    if (queryType == pokemonType) {
                        match = true;
                        break;
                    }
                }
                if (match) break;
            }
            if (match) {
                matches.push_back(&amp;pokemon);
            }
        }

        if (matches.empty()) {
            return "None";
        }

        // Sort by ID
        std::sort(matches.begin(), matches.end(), [](const Pokemon* a, const Pokemon* b) {
            return a-&gt;id &lt; b-&gt;id;
        });

        // Format output
        std::stringstream result;
        result &lt;&lt; matches.size() &lt;&lt; std::endl;
        for (const auto&amp; pokemon : matches) {
            result &lt;&lt; pokemon-&gt;name &lt;&lt; std::endl;
        }
        
        std::string output = result.str();
        // Remove the last newline
        if (!output.empty() &amp;&amp; output.back() == '\n') {
            output.pop_back();
        }
        return output;
    }

    float attack(const char *type, int id) const {
        // Validate type
        if (type == nullptr || !isValidType(type)) {
            throw ArgumentException(("Argument Error: PM Type Invalid (" + std::string(type ? type : "") + ")").c_str());
        }

        // Find the Pokemon
        auto it = pokemons.find(id);
        if (it == pokemons.end()) {
            return -1.0f;
        }

        const Pokemon&amp; pokemon = it-&gt;second;
        
        // Calculate damage multiplier
        float multiplier = 1.0f;
        for (const auto&amp; pokemonType : pokemon.types) {
            auto typeIt = DAMAGE_MULTIPLIER.find(type);
            if (typeIt != DAMAGE_MULTIPLIER.end()) {
                auto damageIt = typeIt-&gt;second.find(pokemonType);
                if (damageIt != typeIt-&gt;second.end()) {
                    multiplier *= damageIt-&gt;second;
                }
            }
        }
        
        return multiplier;
    }

    int catchTry() const {
        if (pokemons.empty()) {
            return 0;
        }

        // Start with the Pokemon with smallest ID
        std::set&lt;int&gt; owned;
        int minId = pokemons.begin()-&gt;first;
        for (const auto&amp; pair : pokemons) {
            if (pair.first &lt; minId) {
                minId = pair.first;
            }
        }
        owned.insert(minId);

        bool progress = true;
        while (progress) {
            progress = false;
            std::vector&lt;int&gt; newCaptures;
            
            // Try to catch all Pokemon not yet owned
            for (const auto&amp; pair : pokemons) {
                int targetId = pair.first;
                if (owned.find(targetId) != owned.end()) {
                    continue;  // Already owned
                }
                
                const Pokemon&amp; target = pair.second;
                bool canCatch = false;
                
                // Check if any owned Pokemon can deal at least 2x damage
                for (int ownedId : owned) {
                    const Pokemon&amp; attacker = pokemons.at(ownedId);
                    
                    // Try each type of the attacker
                    for (const auto&amp; attackerType : attacker.types) {
                        float multiplier = 1.0f;
                        for (const auto&amp; targetType : target.types) {
                            auto typeIt = DAMAGE_MULTIPLIER.find(attackerType);
                            if (typeIt != DAMAGE_MULTIPLIER.end()) {
                                auto damageIt = typeIt-&gt;second.find(targetType);
                                if (damageIt != typeIt-&gt;second.end()) {
                                    multiplier *= damageIt-&gt;second;
                                }
                            }
                        }
                        
                        if (multiplier &gt;= 2.0f) {
                            canCatch = true;
                            break;
                        }
                    }
                    
                    if (canCatch) {
                        break;
                    }
                }
                
                if (canCatch) {
                    newCaptures.push_back(targetId);
                }
            }
            
            // Add newly captured Pokemon
            for (int id : newCaptures) {
                owned.insert(id);
                progress = true;
            }
        }
        
        return owned.size();
    }

    struct iterator {
    private:
        std::map&lt;int, Pokemon&gt;::iterator it;
        const Pokedex* pokedex;

    public:
        iterator(std::map&lt;int, Pokemon&gt;::iterator iter, const Pokedex* p) : it(iter), pokedex(p) {}

        iterator &amp;operator++() {
            ++it;
            return *this;
        }
        
        iterator &amp;operator--() {
            --it;
            return *this;
        }
        
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        
        iterator &amp; operator = (const iterator &amp;rhs) {
            if (this != &amp;rhs) {
                it = rhs.it;
                pokedex = rhs.pokedex;
            }
            return *this;
        }
        
        bool operator == (const iterator &amp;rhs) const {
            return it == rhs.it;
        }
        
        bool operator != (const iterator &amp;rhs) const {
            return it != rhs.it;
        }
        
        Pokemon &amp; operator*() const {
            return it-&gt;second;
        }
        
        Pokemon *operator-&gt;() const {
            return &amp;(it-&gt;second);
        }
    };

    iterator begin() {
        return iterator(pokemons.begin(), this);
    }

    iterator end() {
        return iterator(pokemons.end(), this);
    }
};
