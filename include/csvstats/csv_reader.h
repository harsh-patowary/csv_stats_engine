#pragma once


#include "Vector.h"
#include "Matrix.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <system_error>
#include <charconv>


namespace csv
{
    class CsvError : public std::runtime_error
    {
    public:
        CsvError(int line_num, const std::string &message)
            : std::runtime_error("line " + std::to_string(line_num) + ": " + message) {}
    };

    class CSVReader
    {
    private:
        std::vector<std::string> m_header;
        std::vector<std::vector<std::string>> m_rows;
        std::vector<std::string> split_line(const std::string &line) const;

    public:
        // Constructor reads and validates the file immediately
        explicit CSVReader(const std::string &filepath);

        // Accessors
        const std::vector<std::string> &header() const { return m_header; }
        const std::vector<std::vector<std::string>> &rows() const { return m_rows; }

        bool is_numeric(const std::string& str, double& out_val);
        Matrix<double> parse_numeric_csv(const std::vector<std::vector<std::string>>& raw_rows);
    };
}