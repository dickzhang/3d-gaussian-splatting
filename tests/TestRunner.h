#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace gs::test {

	struct TestCase {
		std::string name;
		std::function<void()> test_fn;
	};

	class TestRunner {
	public:
		static TestRunner& instance() {
			static TestRunner runner;
			return runner;
		}

		void add_test(const std::string& name, std::function<void()> fn) {
			m_tests.push_back({ name, fn });
		}

		int run_all() {
			std::size_t passed = 0;
			std::size_t failed = 0;

			std::cout << "Running " << m_tests.size() << " tests...\n\n";

			for (const auto& test : m_tests) {
				try {
					std::cout << "[TEST] " << test.name << " ... ";
					std::cout.flush();

					test.test_fn();

					std::cout << "PASS\n";
					passed++;
				}
				catch (const std::exception& e) {
					std::cout << "FAIL\n";
					std::cout << "  Error: " << e.what() << "\n";
					failed++;
				}
				catch (...) {
					std::cout << "FAIL\n";
					std::cout << "  Error: Unknown exception\n";
					failed++;
				}
			}

			std::cout << "\n========================================\n";
			std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
			std::cout << "========================================\n";

			return failed > 0 ? 1 : 0;
		}

	private:
		std::vector<TestCase> m_tests;
	};

	// Helper macro for registering tests
#define REGISTER_TEST(name, fn) \
    namespace { \
        struct TestRegistrar_##name { \
            TestRegistrar_##name() { \
                gs::test::TestRunner::instance().add_test(#name, fn); \
            } \
        }; \
        static TestRegistrar_##name registrar_##name; \
    }

	// Assertion helpers
	inline void assert_true(bool condition, const std::string& message = "") {
		if (!condition) {
			throw std::runtime_error(message.empty() ? "Assertion failed" : message);
		}
	}

	inline void assert_equal(std::size_t expected, std::size_t actual, const std::string& context = "") {
		if (expected != actual) {
			std::string msg = context + " - Expected: " + std::to_string(expected) +
				", Actual: " + std::to_string(actual);
			throw std::runtime_error(msg);
		}
	}

	template<typename T>
	inline void assert_equal(T expected, T actual, const std::string& context = "") {
		if (expected != actual) {
			std::string msg = context + " - Values not equal";
			throw std::runtime_error(msg);
		}
	}

} // namespace gs::test
