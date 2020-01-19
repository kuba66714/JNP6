#include <iostream>
#include <unordered_map>
#include <regex>
#include <list>
class Mode {
};
class SequenceMode : public Mode {
};
class OddEvenMode : public Mode {
};
class ShuffleMode : public Mode {
};
SequenceMode createSequenceMode() {
    return SequenceMode();
}
OddEvenMode createOddEvenMode() {
    return OddEvenMode();
}
ShuffleMode createShuffleMode() {
    return ShuffleMode();
}
class PlaylistInterface {
public:
    virtual void play() = 0;
};
class Playlist : public PlaylistInterface {
private:
    std::list<PlaylistInterface*> list_to_play;
    const char* name;
    Mode* mode;
public:
    Playlist(const char* myname) {
        list_to_play = std::list<PlaylistInterface*>();
        name = myname;
        SequenceMode sm;
        mode = &sm;
    }
    void add(PlaylistInterface* pi);
    void add(PlaylistInterface* pi, size_t position);
    void remove();
    void remove(size_t position);
    void setMode(Mode& mode);
    std::list<PlaylistInterface*>& getter() {
        return list_to_play;
    }
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

void Playlist::setMode(Mode& new_mode) {
    mode = &new_mode;
}

class Play : public PlaylistInterface {
private:
    std::unordered_map<std::string, std::string> metadata;
    std::string artist;
    std::string title;
    std::string year;
public:
    Play() = default;
    void play() override = 0;
};
class Song : public Play {
private:
    std::unordered_map<std::string, std::string> metadata;
    std::string artist;
    std::string title;
    std::string lyrics;
public:
    Song(std::unordered_map<std::string, std::string>& data, std::string& new_artist, std::string& new_title, std::string& lyr) {
        metadata = data;
        artist = new_artist;
        title = new_title;
        lyrics = lyr;
    }
    void play() override;
};

void Song::play() {
    std::cout<<"Song ["<<artist<<" "<<title<<"]: "<<lyrics;
}

class Movie : public Play {
private:
    std::unordered_map<std::string, std::string> metadata;
    std::string year;
    std::string title;
    std::string lyrics;
public:
    Movie(std::unordered_map<std::string, std::string>& data, std::string& new_year, std::string& new_title, std::string& lyr) {
        metadata = data;
        year = new_year;
        title = new_title;
        lyrics = lyr;
    }
    void play() override;
};

void Movie::play() {
    std::cout<<"Song ["<<title<<" "<<year<<"]: "<<lyrics;
}

class File {
private:
    std::unordered_map<std::string, std::string> metadata;
    std::string file_type;
    std::string artist;
    std::string title;
    std::string lyrics;
    std::string year;
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
    std::string& get_artist() {
        return artist;
    }
    std::string& get_title() {
        return title;
    }
    std::string& get_lyrics() {
        return lyrics;
    }
    std::string& get_year() {
        return year;
    }
    std::unordered_map<std::string, std::string>& get_metadata() {
        return metadata;
    }
};

void File::parse(std::string &str) {

    std::smatch m;
    std::regex e1("^(audio|video)\\|");
    if (!std::regex_search(str, m, e1)) {
        //BLAD
        return;
    }
    file_type = m.str(0);
    file_type = file_type.substr(0, file_type.size() - 1);
    str = m.suffix().str();
    std::regex e2("([a-zA-Z0-9 ]+):");
    std::regex e3("[^|]*\\|");
    std::regex e4(R"([a-zA-Z0-9\,\.\!\?\'\:\;\-\ ]+)");
    while (std::regex_search(str, m, e2)) {
        std::string data_type = m.str(0);
        data_type = data_type.substr(0, data_type.size() - 1);
        str = m.suffix().str();
        std::regex_search(str, m, e3);
        std::string new_data = m.str(0);
        new_data = new_data.substr(0, new_data.size() - 1);
        if (data_type == "artist") {
            artist = new_data;
        } else if (data_type == "title") {
            title = new_data;
        } else if (data_type == "year") {
            year = new_data;
        } else {
            metadata.insert(std::make_pair(data_type, new_data));
        }
        str = m.suffix().str();
    }
    if (std::regex_match(str, m, e4)) {
        lyrics = str;
    } else {
        //BLAD
    }
    if (lyrics.empty()) {
        //BLAD
    }
    if (file_type == "audio") {
        if (artist.empty() || title.empty()) {
            //BLAD
        }
    } else if (file_type == "video") {
        if (year.empty() || title.empty()) {
            //BLAD
        }
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
    Play* play = new Song(file.get_metadata(), file.get_artist(), file.get_title(), file.get_lyrics());
    return play;
}
class MovieFactory : public PlayFactory {
public:
    MovieFactory() = default;
    Play* create_play(File& file) override ;
};

Play* MovieFactory::create_play(File &file) {
    Play* play =  new Movie(file.get_metadata(), file.get_year(), file.get_title(), file.get_lyrics());
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
    } else {

        //BLAD
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
    File f = File("audio|artist:Louis Armstrong|title:What a Wonderful World|"
                  "I see trees of green, red roses too...");
    auto whatAWonderfulWorld = player.openFile(File("audio|artist:Louis Armstrong|title:What a Wonderful World|"
                                                    "I see trees of green, red roses too..."));
    auto helloDolly = player.openFile(File("audio|artist:Louis Armstrong|title:Hello, Dolly!|"
                                           "Hello, Dolly! This is Louis, Dolly"));
    armstrong->add(whatAWonderfulWorld);
    armstrong->add(helloDolly);
    armstrong->play();
    auto direstraits = player.openFile(File("audio|artist:Dire Straits|title:Money for Nothing|"
                                            "Now look at them yo-yo's that's the way you do it..."));
    auto cabaret = player.openFile(File("video|title:Cabaret|year:1972|Qvfcynlvat Pnonerg"));


    mishmash->add(cabaret);
    mishmash->add(armstrong);
    mishmash->add(direstraits, 1);
    mishmash->add(direstraits);

    std::cout << "=== Playing 'mishmash' (default sequence mode)" << std::endl;
    mishmash->play();

    return 0;
}
