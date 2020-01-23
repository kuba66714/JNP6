
#ifndef ODTWARZACZ_LIB_PLAYLIST_H
#define ODTWARZACZ_LIB_PLAYLIST_H

#include <iostream>
#include <unordered_map>
#include <regex>
#include <list>
#include <random>

class PlayerException : public std::exception{
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
class CorruptFile : public PlayerException {
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
    virtual void play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) {

    }
};
class SequenceMode : public Mode {
public:
    void play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) override;
};

void SequenceMode::play_with_mode(std::list<std::shared_ptr<PlaylistInterface>> &list) {
    std::list<std::shared_ptr<PlaylistInterface>>::iterator it;
    for (it = list.begin(); it != list.end(); it++) {
        (*it)->play();
    }
}

class OddEvenMode : public Mode {
public:
    void play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) override;
};
void play_every_two(std::list<std::shared_ptr<PlaylistInterface>>::iterator& it, std::list<std::shared_ptr<PlaylistInterface>>& list) {
    while (it != list.end()) {
        (*it)->play();
        it++;
        if (it != list.end()) {
            it++;
        }
    }
}

void OddEvenMode::play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) {
    std::list<std::shared_ptr<PlaylistInterface>>::iterator it;
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
    void play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) override;
};

void ShuffleMode::play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) {
    std::vector<std::reference_wrapper<std::shared_ptr<PlaylistInterface>>> vec(list.begin(), list.end());
    std::shuffle(vec.begin(), vec.end(), std::default_random_engine(seed));
    std::list<std::shared_ptr<PlaylistInterface>> new_list {vec.begin(), vec.end()};
    std::list<std::shared_ptr<PlaylistInterface>>::iterator it;
    for (it = new_list.begin(); it != new_list.end(); it++) {
        (*it)->play();
    }
}

std::shared_ptr<SequenceMode> createSequenceMode() {
    return std::make_shared<SequenceMode>();
}
std::shared_ptr<OddEvenMode> createOddEvenMode() {
    return std::make_shared<OddEvenMode>();
}
std::shared_ptr<ShuffleMode> createShuffleMode(size_t seed) {
    return std::make_shared<ShuffleMode>(seed);
}

class Playlist : public PlaylistInterface {
private:
    std::list<std::shared_ptr<PlaylistInterface>> list_to_play;
    const char* name;
    std::shared_ptr<Mode> mode;
public:
    Playlist(const char* myname) {
        list_to_play = std::list<std::shared_ptr<PlaylistInterface>>();
        name = myname;
        std::shared_ptr<SequenceMode> sm = std::make_shared<SequenceMode>();
        mode = sm;
    }
    void add(std::shared_ptr<PlaylistInterface> pi);
    void add(std::shared_ptr<PlaylistInterface> pi, size_t position);
    void remove();
    void remove(size_t position);
    void setMode(std::shared_ptr<Mode> mode);

    void play() override;
};

void Playlist::add(std::shared_ptr<PlaylistInterface> pi) {
    list_to_play.push_back(pi);
    std::shared_ptr<PlaylistInterface> p = list_to_play.front();
}

void Playlist::add(std::shared_ptr<PlaylistInterface> pi, size_t position) {
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

void Playlist::setMode(std::shared_ptr<Mode> new_mode) {
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
    std::string unROT13(std::string &str);
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
        lyrics = unROT13(lyr);
    }
    void play() override;
};

std::string Movie::unROT13(std::string &str) {
    //std::string unROT = str;
    for(auto& it : str) {
        if(it >= 'A' && it <= 'M') it += 13;
        else if(it >= 'N' && it <= 'Z') it -= 13;
        else if(it >= 'a' && it <= 'm') it += 13;
        else if(it >= 'n' && it <= 'z') it -= 13;
    }
    return str;
}

void Movie::play() {
    std::cout<<"Movie ["<<title<<" "<<year<<"]: "<</*unROT13(lyrics)*/lyrics<<std::endl;
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
        if (!std::regex_search(str, m, e3)) {
            throw CorruptFile();
        } else {
            throw WrongType();
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
    virtual std::shared_ptr<Play> create_play(File& file) = 0;
};

class AudioFactory : public PlayFactory {
public:
    AudioFactory() = default;
    std::shared_ptr<Play> create_play(File& file) override ;
};

std::shared_ptr<Play> AudioFactory::create_play(File& file) {
    std::shared_ptr<Play> play = std::make_shared<Song>(file.get_metadata(), file.get_lyrics());
    return play;
}
class MovieFactory : public PlayFactory {
public:
    MovieFactory() = default;
    std::shared_ptr<Play> create_play(File& file) override ;
};

std::shared_ptr<Play> MovieFactory::create_play(File &file) {
    std::shared_ptr<Play> play =  std::make_shared<Movie>(file.get_metadata(), file.get_lyrics());
    return play;
}

class Player {
public:
    static std::shared_ptr<Play> openFile(File file);
    static std::shared_ptr<Playlist> createPlaylist(const char*);
};
std::shared_ptr<Play> Player::openFile(File file) {
    std::shared_ptr<Play> play = nullptr;
    if (file.get_file_type() == "audio") {
        AudioFactory af = AudioFactory();
        play = af.create_play(file);

    } else if (file.get_file_type() == "video") {
        MovieFactory mf = MovieFactory();
        play = mf.create_play(file);
    }
    return play;
}

std::shared_ptr<Playlist> Player::createPlaylist(const char *name) {
    auto playlist = std::make_shared<Playlist>(name);
    return playlist;
}

#endif //ODTWARZACZ_LIB_PLAYLIST_H
