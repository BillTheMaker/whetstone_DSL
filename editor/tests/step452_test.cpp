// Step 452: Concurrency Model Translation Tests (12 tests)

#include "ConcurrencyTranslator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_async_python_to_rust_needs_runtime_review() {
    TEST(async_python_to_rust_needs_runtime_review);
    auto r = ConcurrencyTranslator::analyze("async def f(): await g()", "python", "rust");
    CHECK(r.hasPattern("async/await"), "expected async pattern");
    CHECK(r.countNeedsReview() >= 1, "rust async runtime should require review");
    CHECK(r.translations[0].annotation.find("tokio") != std::string::npos,
          "expected tokio runtime annotation");
    PASS();
}

void test_async_to_c_is_unsafe() {
    TEST(async_to_c_is_unsafe);
    auto r = ConcurrencyTranslator::analyze("async def f(): await g()", "python", "c");
    CHECK(r.hasPattern("async/await"), "expected async pattern");
    CHECK(r.translations[0].safety == ConcurrencySafety::Unsafe, "async->C should be unsafe");
    CHECK(r.translations[0].annotation.find("@Risk(high)") != std::string::npos,
          "expected high risk annotation");
    PASS();
}

void test_thread_java_to_go_maps_to_goroutine() {
    TEST(thread_java_to_go_maps_to_goroutine);
    auto r = ConcurrencyTranslator::analyze("new Thread(() -> {}).start();", "java", "go");
    CHECK(r.hasPattern("thread_spawn"), "expected thread spawn pattern");
    CHECK(r.translations[0].targetCode.find("go func") != std::string::npos,
          "expected goroutine mapping");
    CHECK(r.translations[0].safety == ConcurrencySafety::Safe, "expected safe mapping");
    PASS();
}

void test_thread_to_python_marks_gil_review() {
    TEST(thread_to_python_marks_gil_review);
    auto r = ConcurrencyTranslator::analyze("std::thread t(f);", "cpp", "python");
    CHECK(r.hasPattern("thread_spawn"), "expected thread spawn pattern");
    CHECK(r.translations[0].safety == ConcurrencySafety::NeedsReview, "expected review");
    CHECK(r.translations[0].annotation.find("GIL") != std::string::npos, "expected GIL annotation");
    PASS();
}

void test_channels_go_to_rust_mpsc() {
    TEST(channels_go_to_rust_mpsc);
    auto r = ConcurrencyTranslator::analyze("ch := make(chan int); ch <- 1", "go", "rust");
    CHECK(r.hasPattern("channels"), "expected channels pattern");
    CHECK(r.translations[0].targetCode.find("mpsc::channel") != std::string::npos,
          "expected rust mpsc mapping");
    PASS();
}

void test_mutex_java_to_c_requires_review() {
    TEST(mutex_java_to_c_requires_review);
    auto r = ConcurrencyTranslator::analyze("synchronized(lock) { x++; }", "java", "c");
    CHECK(r.hasPattern("mutex"), "expected mutex pattern");
    CHECK(r.translations[0].safety == ConcurrencySafety::NeedsReview, "expected review in C");
    CHECK(r.translations[0].targetCode.find("pthread_mutex_lock") != std::string::npos,
          "expected pthread mutex mapping");
    PASS();
}

void test_source_model_prefers_async_when_present() {
    TEST(source_model_prefers_async_when_present);
    auto r = ConcurrencyTranslator::analyze(
        "async def f(): await g()\nthreading.Thread(target=h).start()",
        "python",
        "rust");
    CHECK(r.sourceModel == ConcurrencyModel::AsyncAwait, "async should win source model detection");
    PASS();
}

void test_target_model_defaults_for_javascript() {
    TEST(target_model_defaults_for_javascript);
    auto r = ConcurrencyTranslator::analyze("value = 1;", "python", "javascript");
    CHECK(r.targetModel == ConcurrencyModel::EventLoop, "javascript target should default to event loop");
    PASS();
}

void test_no_patterns_on_plain_code() {
    TEST(no_patterns_on_plain_code);
    auto r = ConcurrencyTranslator::analyze("x = a + b;", "python", "rust");
    CHECK(r.translations.empty(), "plain code should not emit translations");
    CHECK(!r.hasPattern("async/await"), "should not detect async pattern");
    PASS();
}

void test_multiple_patterns_detected() {
    TEST(multiple_patterns_detected);
    auto r = ConcurrencyTranslator::analyze(
        "async def f(): await g()\nmutex.lock()\nmutex.unlock()",
        "python",
        "rust");
    CHECK(r.hasPattern("async/await"), "expected async");
    CHECK(r.hasPattern("mutex"), "expected mutex");
    CHECK(r.translations.size() >= 2, "expected multiple translations");
    PASS();
}

void test_async_python_to_python_stays_safe() {
    TEST(async_python_to_python_stays_safe);
    auto r = ConcurrencyTranslator::analyze("async def f(): await g()", "python", "python");
    CHECK(r.hasPattern("async/await"), "expected async pattern");
    CHECK(r.translations[0].safety == ConcurrencySafety::Safe, "python async should stay safe");
    PASS();
}

void test_review_count_aggregates_multiple_translations() {
    TEST(review_count_aggregates_multiple_translations);
    auto r = ConcurrencyTranslator::analyze(
        "async def f(): await g()\nnew Thread(() -> {}).start();",
        "python",
        "java");
    CHECK(r.translations.size() >= 2, "expected async + thread translations");
    CHECK(r.countNeedsReview() >= 1, "expected review-required mappings");
    PASS();
}

int main() {
    std::cout << "Step 452: Concurrency Model Translation Tests\n";

    test_async_python_to_rust_needs_runtime_review();  // 1
    test_async_to_c_is_unsafe();                       // 2
    test_thread_java_to_go_maps_to_goroutine();        // 3
    test_thread_to_python_marks_gil_review();          // 4
    test_channels_go_to_rust_mpsc();                   // 5
    test_mutex_java_to_c_requires_review();            // 6
    test_source_model_prefers_async_when_present();    // 7
    test_target_model_defaults_for_javascript();       // 8
    test_no_patterns_on_plain_code();                  // 9
    test_multiple_patterns_detected();                 // 10
    test_async_python_to_python_stays_safe();          // 11
    test_review_count_aggregates_multiple_translations(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
