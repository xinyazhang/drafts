#include "http_utils.h"

using namespace std;

namespace portal {

    // Auxiliary methods
    void tokenize(string url, const char* delimiter, vector<string> &tokens)
    {
        assert(url.length() > 0);
        assert(delimiter);

        boost::char_separator<char> sep(delimiter);
        boost::tokenizer<boost::char_separator<char> > tok(url, sep);
        for(auto it = tok.begin(); it != tok.end(); ++it){
            tokens.push_back(string(*it));
        }
    }

    const char* filesystem_separator()
    {
    #ifdef _WIN32
        return "\\";
    #else
        return "/";
    #endif
    }

    // API
    string canonicalize_get_url(shared_ptr<HttpServer::Request> req)
    {
        assert(req);

        string url = req->path;
        vector<string> tokens;
        vector<string> canonicalized;
        tokenize(url, filesystem_separator(), tokens);

        // process tokens
        for (auto token : tokens) {
        #if 0
            std::cout << "Token: " << token << std::endl;
        #endif
            if (token == string("..")) {
                if (canonicalized.size() == 0) throw 400; // bad request
                canonicalized.pop_back();
            }
            else if (token == string(".")) {
                // do nothing
            }
            else {
                canonicalized.push_back(token);
            }
        }

        string ret = boost::algorithm::join(canonicalized, "/");
        return string(filesystem_separator()) + ret;
    }

    void render(HttpServer::Response& response, string filename) 
    {
        string webroot(pref::instance()->get_webroot());

        filename = webroot + filename;
        if (boost::filesystem::is_directory(filename)) 
            filename += string(filesystem_separator()) + string("index.html");

		ifstream ifs;
        ifs.open(filename, ifstream::in);

        // consider to make it more flexible, one should retrieve 404 page url 
        // by configuration
        if (!ifs.good()) ifs.open(webroot + string(filesystem_separator()) + string("404.html"), ifstream::in);
        // assert(ifs.good());

        ifs.seekg(0, ios::end);
        size_t length=ifs.tellg();

        ifs.seekg(0, ios::beg);

        response << "HTTP/1.1 200 OK\r\nContent-Length: "
            << length
            << "\r\n\r\n";

        // read and send 128 KB at a time if file-size>buffer_size
        size_t buffer_size=131072;
        if (length > buffer_size) {
            vector<char> buffer(buffer_size);
            size_t read_length;
            while ((read_length=ifs.read(&buffer[0], buffer_size).gcount()) > 0) {
                response.stream.write(&buffer[0], read_length);
                response << HttpServer::flush;
            }
        }
        else
            response << ifs.rdbuf();

        ifs.close();
    }

    void render_bad_request(HttpServer::Response& response)
    {
        render(response, "400.html");
    }

    void render_not_found(HttpServer::Response& response)
    {
        render(response, "404.html");
    }

    void render_internal_server_error(HttpServer::Response& response)
    {
        render(response, "500.html");
    }

};
