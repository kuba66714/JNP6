#include <iostream>
#include <unordered_map>
#include <regex>
#include <list>
#include <random>

class PlayerException : public std::exception{
public:
    const char* what() const noexcept override = 0;
};

class WrongType : public PlayerException {
public:
    const char* what() const noexcept override {
        return "unsupported type";
    }
};
class WrongLyrics : public PlayerException {
public:
    const char* what() const noexcept override {
        return "corrupt content";
    }
};
class NoNecessaryData : public PlayerException {
public:
    const char* what() const noexcept override {
        return "no necessary data";
    }
};
class CorruptedFile : public PlayerException {
public:
    const char* what() const noexcept override {
        return "corrupt file";
    }
};
class PlaylistInterface {
public:
    virtual void play() = 0;
};

class Mode {
public:
    virtual void play_with_mode(std::list<PlaylistInterface*>& list) {

    }
};
class SequenceMode : public Mode {
public:
    void play_with_mode(std::list<PlaylistInterface*>& list) override;
};

void SequenceMode::play_with_mode(std::list<PlaylistInterface *> &list) {
    std::list<PlaylistInterface *>::iterator it;
    for (it = list.begin(); it != list.end(); it++) {
        (*it)->play();
    }
}

class OddEvenMode : public Mode {
public:
    void play_with_mode(std::list<PlaylistInterface*>& list) override;
};
void play_every_two(std::list<PlaylistInterface *>::iterator& it, std::list<PlaylistInterface *>& list) {
    while (it != list.end()) {
        (*it)->play();
        it++;
        if (it != list.end()) {
            it++;
        }
    }
}

void OddEvenMode::play_with_mode(std::list<PlaylistInterface *>& list) {
    std::list<PlaylistInterface *>::iterator it;
    it = list.begin();
    it++;
    play_every_two(it, list);
    it = list.begin();
    play_every_two(it, list);
}

class ShuffleMode : public Mode {
private:
    size_t seed;
public:
    ShuffleMode(size_t new_seed) {
        seed = new_seed;
    }
    void play_with_mode(std::list<PlaylistInterface*>& list) override;
};

void ShuffleMode::play_with_mode(std::list<PlaylistInterface *>& list) {
    std::vector<std::reference_wrapper<PlaylistInterface *>> vec(list.begin(), list.end());
    std::shuffle(vec.begin(), vec.end(), std::default_random_engine(seed));
    std::list<PlaylistInterface*> new_list {vec.begin(), vec.end()};
    std::list<PlaylistInterface *>::iterator it;
    for (it = new_list.begin(); it != new_list.end(); it++) {
        (*it)->play();
    }
}

SequenceMode* createSequenceMode() {
    return new SequenceMode();
}
OddEvenMode* createOddEvenMode() {
    return new OddEvenMode();
}
ShuffleMode* createShuffleMode(size_t seed) {
    return new ShuffleMode(seed);
}

class Playlist : public PlaylistInterface {
private:
    std::list<PlaylistInterface*> list_to_play;
    const char* name;
    Mode* mode;
public:
    Playlist(const char* myname) {
        list_to_play = std::list<PlaylistInterface*>();
        name = myname;
        SequenceMode* sm = new SequenceMode();
        mode = sm;
    }
    void add(PlaylistInterface* pi);
    void add(PlaylistInterface* pi, size_t position);
    void remove();
    void remove(size_t position);
    void setMode(Mode* mode);

    void play() override;
};

void Playlist::add(PlaylistInterface* pi) {
    list_to_play.push_back(pi);
    PlaylistInterface* p = list_to_play.front();
}

void Playlist::add(PlaylistInterface* pi, size_t position) {
    auto it = list_to_play.begin();
    std::advance(it, position);
    list_to_play.insert(it, pi);
}

void Playlist::remove() {
    list_to_play.pop_back();
}

void Playlist::remove(size_t position) {
    auto it = list_to_play.begin();
    std::advance(it, position);
    list_to_play.erase(it);
}

void Playlist::setMode(Mode* new_mode) {
    mode = new_mode;
}

void Playlist::play() {
    std::cout<<"Playlist ["<<name<<"]"<<std::endl;
    mode->play_with_mode(list_to_play);
}

class Play : public PlaylistInterface {

public:
    Play() = default;
    void play() override = 0;
};
class Song : public Play {
private:
    std::string artist;
    std::string title;
    std::string lyrics;
public:
    Song(std::unordered_map<std::string, std::string>& data, std::string& lyrics_add) {
        auto it = data.find("artist");
        if (it == data.end()) {
            throw NoNecessaryData();
        } else {
            artist = it->second;
        }
        it = data.find("title");
        if (it == data.end()) {
            throw NoNecessaryData();
        } else {
            title = it->second;
        }
        lyrics = lyrics_add;
    }
    void play() override;
};

void Song::play() {
    std::cout<<"Song ["<<artist<<" "<<title<<"]: "<<lyrics<<std::endl;
}

class Movie : public Play {
private:
    std::string year;
    std::string title;
    std::string lyrics;
public:
    Movie(std::unordered_map<std::string, std::string>& data, std::string& lyr) {
        auto it = data.find("year");
        if (it == data.end()) {
            throw NoNecessaryData();
        } else {
            year = it->second;
        }
        it = data.find("title");
        if (it == data.end()) {
            throw NoNecessaryData();
        } else {
            title = it->second;
        }
        lyrics = lyr;
    }
    void play() override;
};

void Movie::play() {
    std::cout<<"Movie ["<<title<<" "<<year<<"]: "<<lyrics<<std::endl;
}

class File {
private:
    std::unordered_map<std::string, std::string> metadata;
    std::string file_type;
    std::string lyrics;
    void parse(std::string& str);
public:
     File(const char *str) {
        metadata = std::unordered_map<std::string, std::string>();
        std::string string_str = str;
        parse(string_str);
    }
    std::string& get_file_type() {
         return file_type;
     }

    std::unordered_map<std::string, std::string>& get_metadata() {
        return metadata;
    }
    std::string& get_lyrics() {
         return lyrics;
     }
};

void File::parse(std::string &str) {

    std::smatch m;
    std::regex e1("^(audio|video)\\|");
    std::regex e2("([a-zA-Z0-9 ]+):");
    std::regex e3("[^|]*\\|");
    std::regex e4(R"([a-zA-Z0-9\,\.\!\?\'\:\;\-\ ]+)");
    if (!std::regex_search(str, m, e1)) {
        if (std::regex_search(str, m, e3)) {
            throw WrongType();
        } else {
            throw CorruptedFile();
        }
    }
    file_type = m.str(0);
    file_type = file_type.substr(0, file_type.size() - 1);
    str = m.suffix().str();

    while (std::regex_search(str, m, e2)) {
        std::string data_type = m.str(0);
        data_type = data_type.substr(0, data_type.size() - 1);
        str = m.suffix().str();
        std::regex_search(str, m, e3);
        std::string new_data = m.str(0);
        new_data = new_data.substr(0, new_data.size() - 1);

        metadata.insert(std::make_pair(data_type, new_data));

        str = m.suffix().str();
    }
    if (std::regex_match(str, m, e4)) {
        lyrics = str;
    } else {
        throw WrongLyrics();
    }
}


class PlayFactory {
public:
    virtual Play* create_play(File& file) = 0;
};

class AudioFactory : public PlayFactory {
    public:
    AudioFactory() = default;
    Play* create_play(File& file) override ;
};

Play* AudioFactory::create_play(File& file) {
    Play* play = new Song(file.get_metadata(), file.get_lyrics());
    return play;
}
class MovieFactory : public PlayFactory {
public:
    MovieFactory() = default;
    Play* create_play(File& file) override ;
};

Play* MovieFactory::create_play(File &file) {
    Play* play =  new Movie(file.get_metadata(), file.get_lyrics());
    return play;
}
class Player {
public:
     static Play* openFile(File file);
     static Playlist* createPlaylist(const char*);
};
Play* Player::openFile(File file) {
    Play* play = nullptr;
    if (file.get_file_type() == "audio") {
        AudioFactory af = AudioFactory();
        play = af.create_play(file);

    } else if (file.get_file_type() == "video") {
        MovieFactory mf = MovieFactory();
        play = mf.create_play(file);
    }
    return play;
}

Playlist* Player::createPlaylist(const char *name) {
    auto playlist = new Playlist(name);
    return playlist;
}

int main() {
    Player player;

    auto mishmash = player.createPlaylist("mishmash");
    auto armstrong = player.createPlaylist("armstrong");
    auto whatAWonderfulWorld = player.openFile(File("audio|artist:Louis Armstrong|title:What a Wonderful World|"
                                                    "I see trees of green, red roses too..."));
    auto helloDolly = player.openFile(File("audio|artist:Louis Armstrong|title:Hello, Dolly!|"
                                           "Hello, Dolly! This is Louis, Dolly"));
    armstrong->add(whatAWonderfulWorld);
    armstrong->add(helloDolly);
    auto direstraits = player.openFile(File("audio|artist:Dire Straits|title:Money for Nothing|"
                                            "Now look at them yo-yo's that's the way you do it..."));
    auto cabaret = player.openFile(File("video|title:Cabaret|year:1972|Qvfcynlvat Pnonerg"));


    mishmash->add(cabaret);
    mishmash->add(armstrong);
    mishmash->add(direstraits, 1);
    mishmash->add(direstraits);

    std::cout << "=== Playing 'mishmash' (default sequence mode)" << std::endl;
    mishmash->play();

    std::cout << "=== Playing 'mishmash' (shuffle mode, seed 0 for std::default_random_engine)" << std::endl;
    mishmash->setMode(createShuffleMode(0));
    mishmash->play();

    std::cout << "=== Playing 'mishmash' (removed cabaret and last direstraits, odd-even mode)" << std::endl;
    mishmash->remove(0);
    mishmash->remove();
    mishmash->setMode(createOddEvenMode());
    mishmash->play();

    std::cout << "=== Playing 'mishmash' (sequence mode, 'armstrong' odd-even mode)" << std::endl;
    armstrong->setMode(createOddEvenMode());
    mishmash->setMode(createSequenceMode());
    mishmash->play();

    try {
        auto unsupported = player.openFile(File("mp3|artist:Unsupported|title:Unsupported|Content"));
    } catch (PlayerException const& e) {
        std::cout << e.what() << std::endl;
    }

    try {
        auto corrupted = player.openFile(File("Corrupt"));
    } catch (PlayerException const& e) {
        std::cout << e.what() << std::endl;
    }

    try {
        auto corrupted = player.openFile(File("audio|artist:Louis Armstrong|title:Hello, Dolly!|%#!@*&"));
    } catch (PlayerException const& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
