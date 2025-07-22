#include "histogram.h"

std::ostream& operator<<(std::ostream& os, const histogram& hist)
{
    const std::vector<uint64_t>& counts = hist._counts;

    uint64_t total = 0;
    for (const uint64_t& count : counts)
        total += count;

    const size_t N = counts.size();

    for (size_t i = 0; i < N; i++)
    {
        os << "(" << i << " : " << counts[i] << " ";
        os << 100.0 * ((double) counts[i] / (double) total) << "%";
        os << ")";

        if (i + 1 < N)
            os << '\n';
    }

    return os;
}
