#include <iostream>
#include <unordered_map>
#include <regex>



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
    std::regex e3("([a-zA-Z0-9 ]+)\\|");
    std::regex e4("(.*?)");
    while (std::regex_search(str, m, e2)) {
        std::string data_type = m.str(0);
        data_type = data_type.substr(0, data_type.size() - 1);
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
    std::regex_search(str, m, e4);
    lyrics = m.str(0);
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


class Play {
private:
    std::unordered_map<std::string, std::string> metadata;
    std::string artist;
    std::string title;
    std::string year;
public:
    Play() = default;
};
class Song : public Play {
private:
    std::unordered_map<std::string, std::string> metadata;
    std::string artist;
    std::string title;
public:
    Song(std::unordered_map<std::string, std::string>& data, std::string& new_artist, std::string& new_title) {
        metadata = data;
        artist = new_artist;
        title = new_title;
    }
};
class Movie : public Play {
private:
    std::unordered_map<std::string, std::string> metadata;
    std::string year;
    std::string title;
public:
    Movie(std::unordered_map<std::string, std::string>& data, std::string& new_year, std::string& new_title) {
        metadata = data;
        year = new_year;
        title = new_title;
    }
};
class PlayFactory {
public:
    virtual Play create_play(File& file);
};

Play PlayFactory::create_play(File &file) {
    return Play();
}


class AudioFactory : public PlayFactory {
    public:
    AudioFactory() = default;
    Play create_play(File& file) override ;
};

Play AudioFactory::create_play(File& file) {
    return Song(file.get_metadata(), file.get_artist(), file.get_title());
}
class MovieFactory : public PlayFactory {
public:
    MovieFactory() = default;
    Play create_play(File& file) override ;
};

Play MovieFactory::create_play(File &file) {
    return Movie(file.get_metadata(), file.get_year(), file.get_title());
}
class Player {
public:
    static Play openFile(File& file);
};
Play Player::openFile(File& file) {
    Play play;
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

int main() {
    const char* s = "audio|artist:Dire Straits|title:Money for Nothing|"
                    "Now look at them yo-yo's that's the way you do it...";


    return 0;
}
