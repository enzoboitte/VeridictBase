#ifndef TESTHOTPLUG_IREFLECTABLE_H
#define TESTHOTPLUG_IREFLECTABLE_H

#include <string>
#include <unordered_map>
#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#include <map>
#endif

#include <cstring>
#include <unistd.h>
#include <sys/inotify.h>
#include <atomic>
#include <thread>
namespace fs = std::filesystem;

inline std::atomic<bool> reloadFlag(false);

#include <random>
#include <filesystem>

inline std::string generateRandomString(int length)
{
    const std::string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    std::string random_string;
    for (int i = 0; i < length; ++i) {
        int r = rand() % CHARACTERS.size();
        random_string += CHARACTERS[r];
    }

    return random_string;
}


class HotPlug
{
private:
#if defined(_WIN32)
    HMODULE handle = nullptr;
#else
    void* handle = nullptr;
#endif

    int inotifyFd;
    int wd;
    std::string path;
    std::string path_tmp;
    std::string name;

public:
    HotPlug() = default;
    explicit HotPlug(const char* libraryPath, const char* libraryName, bool watch=true)
    { this->Load(libraryPath, libraryName, watch); };

    ~HotPlug()
    {
#if defined(_WIN32)
        FreeLibrary(this->handle);
#else
        dlclose(this->handle);
#endif
    }

    void Load(const char* libraryPath, const char* libraryName, bool watch=true) {
        std::string temp_name = generateRandomString(100) + ".o";

        this->path     = libraryPath;
        this->name     = libraryName;
        this->path_tmp = "./" + temp_name;

        fs::copy(std::string(libraryPath) + std::string(libraryName), this->path_tmp);
#if defined(_WIN32)
        this->handle = LoadLibraryA(libraryPath);
        if (!this->handle) {
            std::cerr << "Cannot load library: " << GetLastError() << '\n';
            exit(99);
        }
#else
        this->handle = dlopen(this->path_tmp.c_str(), RTLD_NOW);//RTLD_LAZY     -  | RTLD_LOCAL
        if (!this->handle) {
            std::cerr << "Cannot load library: " << dlerror() << '\n';
            exit(99);
        }
        // Reset errors
        dlerror();
#endif
        fs::remove(this->path_tmp);

        if(watch)
            this->LoadWatch();
    }

    void LoadWatch()
    {
        // Setup inotify to watch the directory containing the library
        this->inotifyFd = inotify_init();
        if (this->inotifyFd < 0) {
            std::cerr << "inotify_init failed: " << strerror(errno) << std::endl;
            this->Close();
            exit(EXIT_FAILURE);
        }

        this->wd = inotify_add_watch(this->inotifyFd, this->path.c_str(), IN_CLOSE_WRITE);//".", IN_MODIFY
        if (this->wd < 0) {
            std::cerr << "inotify_add_watch failed: " << strerror(errno) << std::endl;
            close(this->inotifyFd);
            this->Close();
            exit(EXIT_FAILURE);
        }

        std::thread watcher(HotPlug::WatchLibrary, this->inotifyFd, this->name);
        watcher.detach();
    }

    void Reload()
    {
        reloadFlag.store(false);
        this->Close();
        this->Load(this->path.c_str(), this->name.c_str());
    }

    void* GetFunction(const char* functionName)
    {
        if(this->handle == nullptr)
        { exit(99); }

#if defined(_WIN32)
        void* function = (void*)GetProcAddress(this->handle, functionName);
        if (function == nullptr) {
            std::cerr << "Cannot load symbol '" << functionName << "': " << GetLastError() << '\n';
            FreeLibrary(this->handle);
            return nullptr;
        }
#else
        void* function = (void*)dlsym(this->handle, functionName);
        const char *dlsym_error = dlerror();
        if (dlsym_error) {
            std::cerr << "Cannot load symbol '" << functionName << "': " << dlsym_error << '\n';
            dlclose(this->handle);
            return nullptr;
        }
#endif

        return function;
    }

    template<typename T>
    T GetFunction(const char* functionName)
    {
        if(this->handle == nullptr)
        { exit(99); }

#if defined(_WIN32)
        void* function = (void*)GetProcAddress(this->handle, functionName);
        if (function == nullptr) {
            std::cerr << "Cannot load symbol '" << functionName << "': " << GetLastError() << '\n';
            FreeLibrary(this->handle);
            return nullptr;
        }
#else
        T function = (T)dlsym(this->handle, functionName);
        const char *dlsym_error = dlerror();
        if (dlsym_error) {
            std::cerr << "Cannot load symbol '" << functionName << "': " << dlsym_error << '\n';
            dlclose(this->handle);
            return nullptr;
        }
#endif

        return function;
    }

    void *GetHandle() const
    {
        if(this->handle == nullptr)
        { exit(99); }

        return handle;
    }

    void Close()
    {
        if (this->handle != nullptr) {
#if defined(_WIN32)
            FreeLibrary(this->handle);
#else
            if (dlclose(this->handle) != 0) {
                std::cerr << "Cannot close library: " << dlerror() << std::endl;
            }
#endif
            this->handle = nullptr;
        }
    }

    std::string GetPath()
    {
        return this->path;
    }

    std::string GetName()
    {
        return this->name;
    }

    static void WatchLibrary(int inotifyFd, std::string name) {
        const size_t bufLen = 1024;
        char buffer[bufLen];

        while (!reloadFlag.load()) {
            int length = read(inotifyFd, buffer, bufLen);
            if (length < 0) {
                std::cerr << "read failed: " << strerror(errno) << std::endl;
                break;
            }

            for (int i = 0; i < length; i += sizeof(struct inotify_event)) {
                struct inotify_event *event = (struct inotify_event *) &buffer[i];
                std::cout << event->name << std::endl;
                if (strcmp(event->name, name.c_str()) == 0) {
                    std::cout << "Library changed, reloading..." << std::endl;
                    reloadFlag.store(true);
                    break;
                }
            }
        }

        close(inotifyFd);
    }

    bool IsChanged()
    {
        return reloadFlag.load();
    }
};

// Base pour les informations réfléchies d'une propriété
struct PropertyInfo {
    std::string type;
    std::string name;
    void* ptr;
};

// Base de toutes les classes réfléchies
class Reflectable {
public:
    virtual ~Reflectable() = default;
    std::map<std::string, PropertyInfo> properties;

    template<typename T>
    T& GetProperty(const std::string& propName)
    {
        auto propInfo = properties[propName];
        return *reinterpret_cast<T*>(propInfo.ptr);
    }

    void *GetProperty(const std::string& propName)
    {
        auto propInfo = properties[propName];
        return propInfo.ptr;
    }

    template<typename T>
    void SetProperty(const std::string& propName, T value)
    {
        auto propInfo = properties[propName];
        *reinterpret_cast<T*>(propInfo.ptr) = value;
    }

    std::string GetType(const std::string& propName)
    { return properties[propName].type; }
};

typedef struct {
    char** items;     // Tableau de pointeurs vers des chaînes
    size_t size;      // Nombre actuel d'éléments
    size_t capacity;  // Capacité actuelle du tableau
} StringList;

inline void initStringList(StringList* list)
{
    list->size = 0;
    list->capacity = 1; // Commencez avec une capacité de 1 pour simplifier
    list->items = (char**) malloc(sizeof(char*) * list->capacity);
}

inline void push_back(StringList* list, const char* str)
{
    // Vérifier si un redimensionnement est nécessaire
    if (list->size == list->capacity)
    {
        list->capacity *= 2; // Double la capacité
        list->items = (char**) realloc(list->items, sizeof(char*) * list->capacity);
    }

    // Ajouter la nouvelle chaîne
    list->items[list->size] = strdup(str); // Duplique la chaîne et stocke le pointeur
    list->size++;
}
inline void freeStringList(StringList* list)
{
    for (size_t i = 0; i < list->size; i++)
    {
        free(list->items[i]); // Libère chaque chaîne
    }
    free(list->items); // Libère le tableau de pointeurs
}

// Get name classe
#define quote(x) #x

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define MAKE_UNIQUE(x) CONCATENATE(x, __COUNTER__)

#define UFILE() \
inline int G_iNClass   = 0; \
inline StringList G_lClass; \
                \
__attribute__((constructor)) \
void Init_Lib() \
{                 \
    initStringList(&G_lClass); \
}               \
\
extern "C" int GetNbClass() \
{ return G_lClass.size; } \
                \
extern "C" StringList GetListClass() \
{ return G_lClass; }



// Macro pour marquer une classe comme réfléchie
#define UCLASS(C) \
__attribute__((constructor)) \
void MAKE_UNIQUE(C)() \
{                 \
    push_back(&G_lClass, quote(C)); \
    G_iNClass++; \
} \
extern "C" C* CONCATENATE(F_ptr_, C)() \
{ return new C(); }

#define UBODY() \
public: \
    //Reflectable reflectable; \


#define UPROPERTY() \
public:             \
                    \
    template<typename T> \
    void RegisterProperty(const std::string& name, T* ptr) { \
        this->properties[name] = PropertyInfo{typeid(T).name(), name, ptr}; \
    } \
private:

#define CLASS_NAME(C) #C

template<typename T>
using F_rFunc = T (*)();

#endif
