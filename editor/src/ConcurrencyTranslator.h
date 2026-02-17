#pragma once

// Step 452: Concurrency Model Translation — maps concurrency patterns across
// language runtimes with @Exec, @Sync, @ThreadModel, @Blocking annotations.

#include <string>
#include <vector>

enum class ConcurrencyModel { Threads, AsyncAwait, Channels, Coroutines, EventLoop, None };
enum class ConcurrencySafety { Safe, NeedsReview, Unsafe };

struct ConcurrencyTranslation {
    std::string pattern;          // e.g. "async/await", "thread spawn"
    ConcurrencyModel sourceModel;
    ConcurrencyModel targetModel;
    std::string sourceLanguage;
    std::string targetLanguage;
    std::string sourceCode;
    std::string targetCode;
    ConcurrencySafety safety = ConcurrencySafety::Safe;
    std::string annotation;       // e.g. @Blocking(false)
    std::string note;
};

struct ConcurrencyAnalysisResult {
    std::string sourceLanguage;
    std::string targetLanguage;
    ConcurrencyModel sourceModel = ConcurrencyModel::None;
    ConcurrencyModel targetModel = ConcurrencyModel::None;
    std::vector<ConcurrencyTranslation> translations;

    bool hasPattern(const std::string& p) const {
        for (const auto& t : translations)
            if (t.pattern == p) return true;
        return false;
    }

    int countNeedsReview() const {
        int n = 0;
        for (const auto& t : translations)
            if (t.safety == ConcurrencySafety::NeedsReview) ++n;
        return n;
    }
};

class ConcurrencyTranslator {
public:
    static ConcurrencyAnalysisResult analyze(const std::string& source,
                                              const std::string& srcLang,
                                              const std::string& tgtLang) {
        ConcurrencyAnalysisResult result;
        result.sourceLanguage = srcLang;
        result.targetLanguage = tgtLang;
        result.sourceModel = detectModel(source, srcLang);
        result.targetModel = defaultModel(tgtLang);

        if (hasAsync(source)) {
            result.translations.push_back(translateAsync(srcLang, tgtLang));
        }
        if (hasThreads(source)) {
            result.translations.push_back(translateThreads(srcLang, tgtLang));
        }
        if (hasChannels(source)) {
            result.translations.push_back(translateChannels(srcLang, tgtLang));
        }
        if (hasMutex(source)) {
            result.translations.push_back(translateMutex(srcLang, tgtLang));
        }

        return result;
    }

private:
    static bool hasAsync(const std::string& src) {
        return src.find("async ") != std::string::npos ||
               src.find("await ") != std::string::npos ||
               src.find("asyncio") != std::string::npos ||
               src.find("Promise") != std::string::npos;
    }

    static bool hasThreads(const std::string& src) {
        return src.find("Thread") != std::string::npos ||
               src.find("thread") != std::string::npos ||
               src.find("pthread") != std::string::npos ||
               src.find("goroutine") != std::string::npos ||
               src.find("go func") != std::string::npos;
    }

    static bool hasChannels(const std::string& src) {
        return src.find("chan ") != std::string::npos ||
               src.find("channel") != std::string::npos ||
               src.find("mpsc::") != std::string::npos;
    }

    static bool hasMutex(const std::string& src) {
        return src.find("mutex") != std::string::npos ||
               src.find("Mutex") != std::string::npos ||
               src.find("Lock") != std::string::npos ||
               src.find("synchronized") != std::string::npos;
    }

    static ConcurrencyModel detectModel(const std::string& src, const std::string& lang) {
        if (hasAsync(src)) return ConcurrencyModel::AsyncAwait;
        if (hasChannels(src)) return ConcurrencyModel::Channels;
        if (hasThreads(src)) return ConcurrencyModel::Threads;
        if (lang == "go") return ConcurrencyModel::Channels;
        if (lang == "javascript" || lang == "js") return ConcurrencyModel::EventLoop;
        return ConcurrencyModel::None;
    }

    static ConcurrencyModel defaultModel(const std::string& lang) {
        if (lang == "go") return ConcurrencyModel::Channels;
        if (lang == "javascript" || lang == "js") return ConcurrencyModel::EventLoop;
        if (lang == "rust") return ConcurrencyModel::AsyncAwait;
        if (lang == "python") return ConcurrencyModel::AsyncAwait;
        if (lang == "java") return ConcurrencyModel::Threads;
        if (lang == "c" || lang == "cpp") return ConcurrencyModel::Threads;
        return ConcurrencyModel::Threads;
    }

    static ConcurrencyTranslation translateAsync(const std::string& src, const std::string& tgt) {
        ConcurrencyTranslation t;
        t.pattern = "async/await";
        t.sourceModel = ConcurrencyModel::AsyncAwait;
        t.sourceLanguage = src;
        t.targetLanguage = tgt;

        if (tgt == "rust") {
            t.targetModel = ConcurrencyModel::AsyncAwait;
            t.targetCode = "async fn f() -> Result<T> { ... .await }";
            t.safety = ConcurrencySafety::NeedsReview;
            t.annotation = "@Exec(async), @ThreadModel(tokio)";
            t.note = "Rust async requires explicit runtime (tokio/async-std)";
        } else if (tgt == "go") {
            t.targetModel = ConcurrencyModel::Channels;
            t.targetCode = "go func() { ch <- result }()";
            t.safety = ConcurrencySafety::NeedsReview;
            t.annotation = "@Exec(goroutine), @Blocking(false)";
            t.note = "Go uses goroutines+channels instead of async/await";
        } else if (tgt == "python") {
            t.targetModel = ConcurrencyModel::AsyncAwait;
            t.targetCode = "async def f(): result = await coro()";
            t.safety = ConcurrencySafety::Safe;
            t.annotation = "@Exec(async), @ThreadModel(asyncio)";
        } else if (tgt == "java") {
            t.targetModel = ConcurrencyModel::Threads;
            t.targetCode = "CompletableFuture.supplyAsync(() -> { ... })";
            t.safety = ConcurrencySafety::NeedsReview;
            t.annotation = "@Exec(future), @ThreadModel(executor)";
            t.note = "Java uses CompletableFuture, not native async/await";
        } else if (tgt == "c") {
            t.targetModel = ConcurrencyModel::Threads;
            t.targetCode = "pthread_create(&thread, NULL, callback, arg)";
            t.safety = ConcurrencySafety::Unsafe;
            t.annotation = "@Exec(thread), @Blocking(true), @Risk(high)";
            t.note = "C has no async/await — manual thread management";
        } else {
            t.targetModel = ConcurrencyModel::AsyncAwait;
            t.targetCode = "async { ... }";
            t.safety = ConcurrencySafety::NeedsReview;
        }
        return t;
    }

    static ConcurrencyTranslation translateThreads(const std::string& src, const std::string& tgt) {
        ConcurrencyTranslation t;
        t.pattern = "thread_spawn";
        t.sourceModel = ConcurrencyModel::Threads;
        t.sourceLanguage = src;
        t.targetLanguage = tgt;

        if (tgt == "rust") {
            t.targetCode = "std::thread::spawn(move || { ... })";
            t.safety = ConcurrencySafety::Safe;
            t.annotation = "@Sync(Send+Sync), @ThreadModel(native)";
        } else if (tgt == "go") {
            t.targetCode = "go func() { ... }()";
            t.safety = ConcurrencySafety::Safe;
            t.annotation = "@Exec(goroutine)";
        } else if (tgt == "python") {
            t.targetCode = "threading.Thread(target=func).start()";
            t.safety = ConcurrencySafety::NeedsReview;
            t.annotation = "@ThreadModel(GIL), @Blocking(true)";
            t.note = "Python GIL limits true parallelism";
        } else if (tgt == "java") {
            t.targetCode = "new Thread(() -> { ... }).start()";
            t.safety = ConcurrencySafety::Safe;
        } else {
            t.targetCode = "spawn_thread(func)";
            t.safety = ConcurrencySafety::NeedsReview;
        }
        t.targetModel = defaultModel(tgt);
        return t;
    }

    static ConcurrencyTranslation translateChannels(const std::string& src, const std::string& tgt) {
        ConcurrencyTranslation t;
        t.pattern = "channels";
        t.sourceModel = ConcurrencyModel::Channels;
        t.sourceLanguage = src;
        t.targetLanguage = tgt;

        if (tgt == "rust") {
            t.targetCode = "let (tx, rx) = mpsc::channel(); tx.send(value);";
            t.safety = ConcurrencySafety::Safe;
            t.annotation = "@Sync(channel)";
        } else if (tgt == "go") {
            t.targetCode = "ch := make(chan T); ch <- value";
            t.safety = ConcurrencySafety::Safe;
        } else if (tgt == "java") {
            t.targetCode = "BlockingQueue<T> queue = new LinkedBlockingQueue<>();";
            t.safety = ConcurrencySafety::Safe;
        } else {
            t.targetCode = "channel.send(value)";
            t.safety = ConcurrencySafety::NeedsReview;
        }
        t.targetModel = defaultModel(tgt);
        return t;
    }

    static ConcurrencyTranslation translateMutex(const std::string& src, const std::string& tgt) {
        ConcurrencyTranslation t;
        t.pattern = "mutex";
        t.sourceModel = ConcurrencyModel::Threads;
        t.sourceLanguage = src;
        t.targetLanguage = tgt;

        if (tgt == "rust") {
            t.targetCode = "let data = Mutex::new(value); let guard = data.lock().unwrap();";
            t.safety = ConcurrencySafety::Safe;
            t.annotation = "@Sync(Mutex)";
        } else if (tgt == "go") {
            t.targetCode = "var mu sync.Mutex; mu.Lock(); defer mu.Unlock();";
            t.safety = ConcurrencySafety::Safe;
        } else if (tgt == "java") {
            t.targetCode = "synchronized(lock) { ... }";
            t.safety = ConcurrencySafety::Safe;
        } else if (tgt == "c") {
            t.targetCode = "pthread_mutex_lock(&mutex); ... pthread_mutex_unlock(&mutex);";
            t.safety = ConcurrencySafety::NeedsReview;
            t.annotation = "@Sync(pthread_mutex)";
        } else {
            t.targetCode = "lock.acquire(); ... lock.release();";
            t.safety = ConcurrencySafety::NeedsReview;
        }
        t.targetModel = defaultModel(tgt);
        return t;
    }
};
