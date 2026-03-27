// Minimal test template for chunk-driven dispatch
// Replace with actual implementation when ChunkDispatchScheduler is created

#include "TestRunner.h"
#include <algorithm>
#include <vector>

namespace gs::test {

	struct ChunkInfo {
		std::uint32_t start_index;
		std::uint32_t splat_count;
	};

	struct DispatchRange {
		std::uint32_t start;
		std::uint32_t count;
	};

	// TODO: Replace with actual ChunkDispatchScheduler implementation
	class ChunkDispatchSchedulerStub {
	public:
		constexpr static std::uint32_t kWorkgroupSize = 256;

		std::vector<DispatchRange> computeDispatchRanges(const std::vector<ChunkInfo>& chunks) {
			std::vector<DispatchRange> ranges;
			if (chunks.empty()) return ranges;

			// Simple implementation: merge contiguous chunks
			DispatchRange current{ chunks[0].start_index, chunks[0].splat_count };

			for (std::size_t i = 1; i < chunks.size(); i++) {
				const auto& chunk = chunks[i];
				if (chunk.start_index == current.start + current.count) {
					// Contiguous: merge
					current.count += chunk.splat_count;
				}
				else {
					// Gap: finalize current range and start new one
					ranges.push_back(current);
					current = { chunk.start_index, chunk.splat_count };
				}
			}
			ranges.push_back(current);

			return ranges;
		}

		std::uint32_t computeWorkgroups(std::uint32_t splat_count) const {
			if (splat_count == 0) return 0;
			return (splat_count + kWorkgroupSize - 1) / kWorkgroupSize;
		}
	};

	void test_dispatch_range_single_chunk() {
		ChunkDispatchSchedulerStub scheduler;

		std::vector<ChunkInfo> chunks = { {0, 100} };
		auto ranges = scheduler.computeDispatchRanges(chunks);

		assert_equal<std::size_t>(1, ranges.size(), "Single chunk should produce one range");
		assert_equal<std::uint32_t>(0, ranges[0].start, "Range start");
		assert_equal<std::uint32_t>(100, ranges[0].count, "Range count");
	}

	void test_dispatch_range_contiguous_chunks() {
		ChunkDispatchSchedulerStub scheduler;

		std::vector<ChunkInfo> chunks = {
			{0, 100},
			{100, 150}  // Contiguous: should merge
		};
		auto ranges = scheduler.computeDispatchRanges(chunks);

		assert_equal<std::size_t>(1, ranges.size(), "Contiguous chunks should merge into one range");
		assert_equal<std::uint32_t>(0, ranges[0].start, "Merged range start");
		assert_equal<std::uint32_t>(250, ranges[0].count, "Merged range count");
	}

	void test_dispatch_range_non_contiguous() {
		ChunkDispatchSchedulerStub scheduler;

		std::vector<ChunkInfo> chunks = {
			{0, 100},
			{500, 100}  // Gap: should NOT merge
		};
		auto ranges = scheduler.computeDispatchRanges(chunks);

		assert_equal<std::size_t>(2, ranges.size(), "Non-contiguous chunks should produce separate ranges");
		assert_equal<std::uint32_t>(0, ranges[0].start, "First range start");
		assert_equal<std::uint32_t>(100, ranges[0].count, "First range count");
		assert_equal<std::uint32_t>(500, ranges[1].start, "Second range start");
		assert_equal<std::uint32_t>(100, ranges[1].count, "Second range count");
	}

	void test_workgroup_sizing() {
		ChunkDispatchSchedulerStub scheduler;

		assert_equal<std::uint32_t>(0, scheduler.computeWorkgroups(0), "Zero splats");
		assert_equal<std::uint32_t>(1, scheduler.computeWorkgroups(1), "One splat");
		assert_equal<std::uint32_t>(1, scheduler.computeWorkgroups(256), "Exactly one workgroup");
		assert_equal<std::uint32_t>(2, scheduler.computeWorkgroups(257), "Just over one workgroup");
		assert_equal<std::uint32_t>(2, scheduler.computeWorkgroups(512), "Exactly two workgroups");
		assert_equal<std::uint32_t>(4, scheduler.computeWorkgroups(1024), "Four workgroups");
	}

	void test_empty_chunk_list() {
		ChunkDispatchSchedulerStub scheduler;

		std::vector<ChunkInfo> chunks;
		auto ranges = scheduler.computeDispatchRanges(chunks);

		assert_equal<std::size_t>(0, ranges.size(), "Empty chunk list should produce no ranges");
	}

	REGISTER_TEST(DispatchRangeSingleChunk, test_dispatch_range_single_chunk);
	REGISTER_TEST(DispatchRangeContiguous, test_dispatch_range_contiguous_chunks);
	REGISTER_TEST(DispatchRangeNonContiguous, test_dispatch_range_non_contiguous);
	REGISTER_TEST(WorkgroupSizing, test_workgroup_sizing);
	REGISTER_TEST(EmptyChunkList, test_empty_chunk_list);

} // namespace gs::test

int main() {
	std::cout << "=== Chunk Dispatch Tests ===\n\n";
	return gs::test::TestRunner::instance().run_all();
}
