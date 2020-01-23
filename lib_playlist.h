//
// Created by kuba on 23.01.2020.
//
#ifndef JNP6_LIB_PLAYLIST_H
#define JNP6_LIB_PLAYLIST_H
#include <iostream>
#include <unordered_map>
#include <regex>
#include <list>
#include <random>
//Korzen klas wyjatkow
class PlayerException : public std::exception{
};
//Wyjatek, gdy zostanie podany zly typ
class WrongType : public PlayerException {
public:
    const char* what() const noexcept override {
        return "unsupported type";
    }
};
//Wyjatek, gdy nie zostanie podany tekst
class WrongLyrics : public PlayerException {
public:
    const char* what() const noexcept override {
        return "corrupt content";
    }
};
//wyjatek, gdy nie zostana podane szytskie potrzebne dane
class NoNecessaryData : public PlayerException {
public:
    const char* what() const noexcept override {
        return "no necessary data";
    }
};
//wyjatek, gdy zostanie podany zly format pliku
class CorruptFile : public PlayerException {
public:
    const char* what() const noexcept override {
        return "corrupt file";
    }
};
//wyjatek, gdy playlisty by sie zapetlily
class NoCyclesAllowed : public PlayerException {
public:
    const char* what() const noexcept override {
        return "cycles not allowed";
    }
};
//wyjatek, gdy rok nie jest liczba
class WrongYear : public PlayerException {
public:
    const char* what() const noexcept override {
        return "wrong year";
    }
};
class RemoveError : public PlayerException {
public:
    const char* what() const noexcept override {
        return "remove error";
    }
};
//Abstrakcyjna klasa playlisty
class PlaylistInterface {
public:
    virtual void play() = 0;
    virtual bool is_collision(PlaylistInterface* obj) = 0;
    virtual bool can_cause_collision() = 0;

    virtual ~PlaylistInterface() = default;
};
//Abstrakcyjna klasa sposobu odtwarzania
class Mode {
public:
    virtual void play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) = 0;
    virtual ~Mode() = default;
};
//sekwencyjny sposob odtwarzania
class SequenceMode : public Mode {
public:
    void play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) override;
};
//metoda, ktora odtwarza playliste w kolejnosci sekwencyjnej
void SequenceMode::play_with_mode(std::list<std::shared_ptr<PlaylistInterface>> &list) {
    std::list<std::shared_ptr<PlaylistInterface>>::iterator it;
    for (it = list.begin(); it != list.end(); it++) {
        (*it)->play();
    }
}
//sposob odtwarzania nieparzyste/parzyste
class OddEvenMode : public Mode {
public:
    void play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) override;
};
//metoda, ktora odtwarza co druga piosenke
void play_every_two(std::list<std::shared_ptr<PlaylistInterface>>::iterator& it, std::list<std::shared_ptr<PlaylistInterface>>& list) {
    while (it != list.end()) {
        (*it)->play();
        it++;
        if (it != list.end()) {
            it++;
        }
    }
}
//metoda, ktora odtwarza playliste w kolejnosci nieparzyste/parzyste
void OddEvenMode::play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) {
    std::list<std::shared_ptr<PlaylistInterface>>::iterator it;
    it = list.begin();
    it++;
    play_every_two(it, list);
    it = list.begin();
    play_every_two(it, list);
}
//sposob odtwarzania losowy
class ShuffleMode : public Mode {
private:
    size_t seed;
public:
    ShuffleMode(size_t new_seed) {
        seed = new_seed;
    }
    void play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) override;
};
//metoda, ktora odtwarza w kolejnosci losowej
void ShuffleMode::play_with_mode(std::list<std::shared_ptr<PlaylistInterface>>& list) {
    std::vector<std::reference_wrapper<std::shared_ptr<PlaylistInterface>>> vec(list.begin(), list.end());
    std::shuffle(vec.begin(), vec.end(), std::default_random_engine(seed));
    std::list<std::shared_ptr<PlaylistInterface>> new_list {vec.begin(), vec.end()};
    std::list<std::shared_ptr<PlaylistInterface>>::iterator it;
    for (it = new_list.begin(); it != new_list.end(); it++) {
        (*it)->play();
    }
}
//metoda, ktora zwraca klase reprezentujaca sekwencyjna
//kolejnosc odtwarzania
std::shared_ptr<SequenceMode> createSequenceMode() {
    return std::make_shared<SequenceMode>();
}
//metoda, ktora zwraca klase reprezentujaca
//kolejnosc odtwarzania nieparzyste/parzyste
std::shared_ptr<OddEvenMode> createOddEvenMode() {
    return std::make_shared<OddEvenMode>();
}
//metoda, ktora zwraca klase reprezentujaca losowa
//kolejnosc odtwarzania
std::shared_ptr<ShuffleMode> createShuffleMode(size_t seed) {
    return std::make_shared<ShuffleMode>(seed);
}
//klasa Playlisty reprezentowanej, jako liste klas PlaylistInterface
class Playlist : public PlaylistInterface {
private:
    std::list<std::shared_ptr<PlaylistInterface>> list_to_play;
    //nazwa playlisty
    const char* name;
    //sposob odtwarzania
    std::shared_ptr<Mode> mode;
public:
    Playlist(const char* myname) {
        list_to_play = std::list<std::shared_ptr<PlaylistInterface>>();
        name = myname;
        std::shared_ptr<SequenceMode> sm = std::make_shared<SequenceMode>();
        mode = sm;
    }
    void add(const std::shared_ptr<PlaylistInterface>& pi);
    void add(const std::shared_ptr<PlaylistInterface>& pi, size_t position);
    void remove();
    void remove(size_t position);
    void setMode(std::shared_ptr<Mode> mode);
    bool is_collision(PlaylistInterface* obj) override;
    bool can_cause_collision() override;
    void play() override;
};
//dodaje nowy element do playlisty
void Playlist::add(const std::shared_ptr<PlaylistInterface>& pi) {
    if (pi->is_collision(this)) {
        throw NoCyclesAllowed();
    }
    list_to_play.push_back(pi);
}
//dodaje nowy element, na konkretna pozycje w liscie
void Playlist::add(const std::shared_ptr<PlaylistInterface>& pi, size_t position) {
    if (pi->is_collision(this)) {
        throw NoCyclesAllowed();
    }
    auto it = list_to_play.begin();
    std::advance(it, position);
    list_to_play.insert(it, pi);
}
//usuwa ostatni element
void Playlist::remove() {
    if (!list_to_play.empty()) {
        list_to_play.pop_back();
    }
    else {
        throw RemoveError();
    }
}
//usuwa element z okreslonej pozycji
//lub rzuca wyjatek, gdy pozycja jest niepoprawna
void Playlist::remove(size_t position) {
    if (position > list_to_play.size()) {
        throw RemoveError();
    }
    auto it = list_to_play.begin();
    std::advance(it, position);
    list_to_play.erase(it);
}
//ustawia nowa metode odtwarzania
void Playlist::setMode(std::shared_ptr<Mode> new_mode) {
    mode = std::move(new_mode);
}
//odtwarza, wedlug ustawionego sposobu
void Playlist::play() {
    std::cout<<"Playlist ["<<name<<"]"<<std::endl;
    mode->play_with_mode(list_to_play);
}
//sprawdza czy playlisty nie tworza cykli
bool Playlist::is_collision(PlaylistInterface* obj) {
    if (obj == this) {
        return true;
    }
    auto it = list_to_play.begin();
    while (it != list_to_play.end()) {
        if ((*it)->can_cause_collision()) {
            Playlist* pl = dynamic_cast<Playlist*>(it->get());
            bool b1 = (this == obj);
            bool b2 = pl->is_collision(obj);
            if (b1|| b2) {
                return true;
            }
        }
        it++;
    }
    return false;
}
//metoda okreslajaca, czy dana klasa moze powodowac kolizje
bool Playlist::can_cause_collision() {
    return true;
}
//abstarkcyjna klasa Play, do ktorej naleza
// Song i Movie
class Play : public PlaylistInterface {

public:
    Play() = default;
    bool is_collision(PlaylistInterface* obj) override;
    bool can_cause_collision() override;
    void play() override = 0;
};
//Obiekty tej klasy nie moga powodowac kolizji w postaci cykli
bool Play::is_collision(PlaylistInterface* obj) {
    (void)obj;
    return false;
}
//Obiekty tej klasy nie moga powodowac kolizji w postaci cykli
bool Play::can_cause_collision() {
    return false;
}
//Klasa reprezentujaca piosenke
class Song : public Play {
private:
    std::string artist;
    std::string title;
    std::string lyrics;
public:
    Song(std::unordered_map<std::string, std::string>& data, std::string& lyrics_add);
    void play() override;
};
//konstruktor klasy piosenka, ktory sprawdza czy wszytskie parametry sa podane poprawnie
Song::Song(std::unordered_map<std::string, std::string>& data, std::string& lyrics_add) {
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
//metoda odtwarzajaca piosenke
void Song::play() {
    std::cout<<"Song ["<<artist<<" "<<title<<"]: "<<lyrics<<std::endl;
}
//metoda sprawdza, czy podano poprawny format roku
bool correct_year(std::string& year) {

    for (char const &c : year) {
        if (c < '0' || c > '9')
            return false;
    }
    return true;
}
//Klasa reprezentujaca film
class Movie : public Play {
private:
    std::string year;
    std::string title;
    std::string lyrics;
    std::string unROT13(std::string &str);
public:
    Movie(std::unordered_map<std::string, std::string>& data, std::string& lyr);
    void play() override;
};
//Konstruktor klasy Movie, ktory sprawdza czy wszytkie parametry zostaly podane poprawnie
Movie::Movie(std::unordered_map<std::string, std::string>& data, std::string& lyr) {
    auto it = data.find("year");
    if (it == data.end()) {
        throw NoNecessaryData();
    } else {
        std::string& new_year = it->second;
        if (correct_year(new_year)) {
            year = it->second;
        } else {
            throw WrongYear();
        }
    }
    it = data.find("title");
    if (it == data.end()) {
        throw NoNecessaryData();
    } else {
        title = it->second;
    }
    lyrics = unROT13(lyr);
}
//Metoda, ktora deszyfruje ROT13
std::string Movie::unROT13(std::string &str) {
    for(auto& it : str) {
        if(it >= 'A' && it <= 'M') it += 13;
        else if(it >= 'N' && it <= 'Z') it -= 13;
        else if(it >= 'a' && it <= 'm') it += 13;
        else if(it >= 'n' && it <= 'z') it -= 13;
    }
    return str;
}
//metoda, ktora odtwarza film
void Movie::play() {
    std::cout<<"Movie ["<<title<<" "<<year<<"]: "<<lyrics<<std::endl;
}
//Klasa reprezentujaca plik i jego metadane
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
//metoda, ktora pasrduje nazwe pliku i wydziela metadane
void File::parse(std::string& str) {
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
//Abstrakcyjna metoda, reprezentujaca klasy, ktore
//tworza nowe obiekty klas
class PlayFactory {
public:
    virtual std::shared_ptr<Play> create_play(File& file) = 0;

    virtual ~PlayFactory() = default;
};
//Twory obiekt klasy Song
class AudioFactory : public PlayFactory {
public:
    AudioFactory() = default;
    std::shared_ptr<Play> create_play(File& file) override ;
};
//metoda tworzaca nowy obiekt klasy Song
std::shared_ptr<Play> AudioFactory::create_play(File& file) {
    std::shared_ptr<Play> play = std::make_shared<Song>(file.get_metadata(), file.get_lyrics());
    return play;
}
//klasa tworzaca nowy obiekt klasy Movie
class MovieFactory : public PlayFactory {
public:
    MovieFactory() = default;
    std::shared_ptr<Play> create_play(File& file) override ;
};
//metoda tworzaca nowy obiekt klasy Movie
std::shared_ptr<Play> MovieFactory::create_play(File &file) {
    std::shared_ptr<Play> play =  std::make_shared<Movie>(file.get_metadata(), file.get_lyrics());
    return play;
}
//Klasa reprezentujaca Player
class Player {
public:
    static std::shared_ptr<Play> openFile(File file);
    static std::shared_ptr<Playlist> createPlaylist(const char*);
};
//metoda, ktora zleca stworznie nowego obiektu klasy Play
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
//metoda tworzaca nowa Playliste
std::shared_ptr<Playlist> Player::createPlaylist(const char *name) {
    auto playlist = std::make_shared<Playlist>(name);
    return playlist;
}

#endif //JNP6_LIB_PLAYLIST_H
