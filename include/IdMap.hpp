#ifndef IDMAP_HPP
#define IDMAP_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

using namespace std;

#define DEFAULT_CHUNK_SIZE 1024

// Possible chunk status
#define CHUNK_NOT_INIT 0
#define CHUNK_OPENED 1
#define CHUNK_BEING_USED 2
#define CHUNK_FULL 3

struct Chunk {
    int id;
    int status;
    int start;
    int end;
    Chunk(): id(0), status(CHUNK_NOT_INIT), start(0), end(0) {}
};

struct FileLocker {
    int fd;
    FileLocker(string file) {
        fd = open((file + ".lock").c_str(), O_RDWR|O_CREAT, 0666);
        if (flock(fd, LOCK_EX) != 0) cerr << "Error while locking " << file << ".lock fd=" << fd << endl;
    }
    ~FileLocker() {
        if (flock(fd, LOCK_UN) != 0) cerr << "Error while unlocking fd=" << fd << endl;
        close(fd);
    }
};

struct IdMap {
    map<string, int> id_map;
    map<string, int> new_id_map;
    Chunk curchunk;
    
    string fname;
    string fchunkspool;
    int flockd;

    IdMap(string filename) {
        const char * val = ::getenv("DIN_MAPS");
        string prefix = "./";
        if ((val == 0) || (strcmp(val,"") == 0)) {
            cerr << "DIN_MAPS not set, falling back to current directory" << endl;
        } else {
            prefix = val;
            prefix += "/";
        }

        fname = prefix + filename;
        fchunkspool = fname + ".chunkspool.json";

        readMap();
        curchunk = Chunk();
        reserveNewChunk();
    }

    ~IdMap() {
        recycleCurChunk();
    }

    void readMap() { // thread safe
        FileLocker lock(fname);
        readMapFromJson(id_map, fname);
    }


    void saveMap() { // thread safe
        FileLocker lock(fname);
        readMapFromJson(id_map, fname);
        id_map.insert(new_id_map.begin(), new_id_map.end());
        writeMapToJson(id_map, fname);
    }

    int getId(string src) {
        if (id_map.count(src) != 0) return id_map[src];
        if (new_id_map.count(src) != 0) return new_id_map[src];

        if (curchunk.start > curchunk.end) {
            reserveNewChunk();
        }
        int new_id = curchunk.start++;
        new_id_map[src] = new_id;
        return new_id;
    }

    inline string s(int i) {
        char buf[33];
        sprintf(buf, "%d", i);
        return string(buf);
    }

    inline void putChunk(map<string, int> &pool, Chunk &chk) {
        string name = "chunk_" + s(chk.id);
        pool[name + "_status"] = chk.status;
        pool[name + "_start"] = chk.start;
        pool[name + "_end"] = chk.end;
    }

    inline void getChunk(map<string, int> &pool, int id, Chunk &chk) {
        string name = "chunk_" + s(id);
        chk.id = id;
        chk.status = pool[name + "_status"];
        chk.start = pool[name + "_start"];
        chk.end = pool[name + "_end"];
    }

    void reserveNewChunk() { // thread safe
        FileLocker lock(fchunkspool);

        map<string, int> pool;
        readMapFromJson(pool, fchunkspool);

        if (pool.count("size_of_each_chunk") == 0) {
            pool["size_of_each_chunk"] = DEFAULT_CHUNK_SIZE;
        }
        if (pool.count("latest_chunk_id") == 0) {
            pool["latest_chunk_id"] = -1;
        }

        // update current chunk in the pool
        if (curchunk.status != CHUNK_NOT_INIT) {
            curchunk.status = curchunk.start > curchunk.end ? CHUNK_FULL : CHUNK_OPENED;
            putChunk(pool, curchunk);
        }

        // try to find an opened chunk (the chunk that is not fully used)
        bool foundOpenChunk = false;
        for (int i = 0; i <= pool["latest_chunk_id"]; i++) {
            getChunk(pool, i, curchunk);
            if (curchunk.status == CHUNK_OPENED) {
                curchunk.status = CHUNK_BEING_USED;
                putChunk(pool, curchunk);
                foundOpenChunk = true;
                break;
            }
        }

        if (!foundOpenChunk) {
            pool["latest_chunk_id"] += 1;
            curchunk.id = pool["latest_chunk_id"];
            curchunk.status = CHUNK_BEING_USED;
            curchunk.start = curchunk.id * pool["size_of_each_chunk"];
            curchunk.end = curchunk.start + pool["size_of_each_chunk"] - 1;
            putChunk(pool, curchunk);
        }

        writeMapToJson(pool, fchunkspool);
    }

    void recycleCurChunk() { // thread safe
        FileLocker lock(fchunkspool);

        map<string, int> pool;
        readMapFromJson(pool, fchunkspool);

        // recycle the chunk if it is not fully used
        curchunk.status = curchunk.start > curchunk.end ? CHUNK_FULL : CHUNK_OPENED;
        putChunk(pool, curchunk);

        writeMapToJson(pool, fchunkspool);
    }

    bool readMapFromJson(map<string, int> &mp, string jsonfile) {
        ifstream mapin;
        mapin.open(jsonfile);
        if (!mapin.is_open()) {
            return false;
        }
        stringstream ss;
        string err;
        ss << mapin.rdbuf();
        json11::Json jsmap;
        jsmap = json11::Json::parse(ss.str(), err);

        for (auto entry : jsmap.object_items()) {
            mp[entry.first] = entry.second.int_value();
        }
        return true;
    }

    bool writeMapToJson(map<string, int> &mp, string jsonfile) {
        ofstream mapout;
        mapout.open(jsonfile, ofstream::trunc);
        json11::Json jsmap(mp);
        mapout << jsmap.dump();
        mapout.flush();
        mapout.close();
        return true;
    }
};


#endif
