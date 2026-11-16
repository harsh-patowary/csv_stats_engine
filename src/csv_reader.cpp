#include "csvstats/csv_reader.h"
#include <iostream>

namespace csv
{

    std::vector<std::string> CSVReader::split_line(const std::string &line) const
    {
        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;

        while (std::getline(ss, field, ','))
        {
            fields.push_back(field);
        }

        // Handle trailing comma case (e.g., "a,b," should produce 3 fields, not 2)
        if (!line.empty() && line.back() == ',')
        {
            fields.push_back("");
        }

        return fields;
    }

    CSVReader::CSVReader(const std::string &filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            throw std::runtime_error("Error: Could not open file: " + filepath);
        }

        std::string line;
        int line_number = 0;

        // 1. Read the header row
        if (std::getline(file, line))
        {
            line_number++;
            // Strip trailing '\r' if present (Windows line endings)
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }
            m_header = split_line(line);
        }
        else
        {
            throw std::runtime_error("Error: CSV file is empty.");
        }

        std::size_t expected_fields = m_header.size();

        // 2. Read data rows
        while (std::getline(file, line))
        {
            line_number++;

            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            // Skip completely empty lines
            if (line.empty())
                continue;

            std::vector<std::string> row = split_line(line);

            // Enforce strict field count matching
            if (row.size() != expected_fields)
            {
                throw CsvError(line_number, "expected " + std::to_string(expected_fields) +
                                                " fields, got " + std::to_string(row.size()));
            }

            m_rows.push_back(row);
        }
    }

    bool CSVReader::is_numeric(const std::string &str, double &out_val)
    {
        if (str.empty())
            return false;

        const char *first = str.data();
        const char *last = str.data() + str.size();
        auto results = std::from_chars(first, last, out_val);

        return results.ec == std::errc() && results.ptr == last;
    }

    Matrix<double> CSVReader::parse_numeric_csv(const std::vector<std::vector<std::string>> &raw_rows)
    {
        if (raw_rows.empty() || raw_rows[0].empty())
        {
            throw std::runtime_error("CSV data is empty.");
        }

        std::size_t total_rows = raw_rows.size();
        std::size_t total_cols = raw_rows[0].size();

        // Step 1: Filter out columns that are entirely non-numeric (like text/categorical columns)
        std::vector<std::size_t> valid_col_indices;

        for (std::size_t c = 0; c < total_cols; ++c)
        {
            std::size_t numeric_count = 0;
            for (std::size_t r = 0; r < total_rows; ++r)
            {
                double dummy = 0.0;
                if (is_numeric(raw_rows[r][c], dummy))
                {
                    numeric_count++;
                }
            }

            // Rule: If at least 50% (or a threshold) of the column is numeric, keep it as a numeric feature.
            // (Adjust this threshold based on your dataset requirements)
            if (numeric_count > 0)
            {
                valid_col_indices.push_back(c);
            }
            else
            {
                std::cerr << "[INFO] Excluding entirely non-numeric column " << c << ".\n";
            }
        }

        if (valid_col_indices.empty())
        {
            throw std::runtime_error("No valid numeric columns found in CSV data!");
        }

        // Step 2: Allocate the custom Matrix for valid columns
        std::size_t numeric_rows = total_rows;
        std::size_t numeric_cols = valid_col_indices.size();

        Matrix<double> numeric_matrix(numeric_rows, numeric_cols);

        // Step 3: Populate with Graceful Imputation for malformed/missing cells
        for (std::size_t r = 0; r < numeric_rows; ++r)
        {
            for (std::size_t new_c = 0; new_c < numeric_cols; ++new_c)
            {
                std::size_t original_c = valid_col_indices[new_c];
                const std::string &cell_str = raw_rows[r][original_c];

                double val = 0.0;
                const char *first = cell_str.data();
                const char *last = cell_str.data() + cell_str.size();

                auto [ptr, ec] = std::from_chars(first, last, val);

                // If the cell is empty or malformed, handle it gracefully instead of crashing
                if (ec != std::errc() || ptr != last)
                {
                    val = 0.0; // Imputation fallback value (or NAN)
                    std::cerr << "[WARNING] Row " << r << ", Col " << original_c
                              << " (\"" << cell_str << "\"): Invalid numeric format. Imputing 0.0.\n";
                }

                numeric_matrix(r, new_c) = val;
            }
        }

        std::cout << "[INFO] Successfully parsed CSV: Kept " << numeric_cols
                  << " numeric columns out of " << total_cols << " total columns with imputation enabled.\n";

        return numeric_matrix;
    }

} // namespace csv