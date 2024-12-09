#define DUCKDB_EXTENSION_MAIN
#include "tsid_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/common/exception/conversion_exception.hpp"
#include "uutid.hpp"

namespace duckdb {

// Generate new TSID (with input parameter - ignored)
static void TsidScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
    auto result_data = FlatVector::GetData<string_t>(result);
    auto &validity = FlatVector::Validity(result);
    
    for (idx_t i = 0; i < args.size(); i++) {
        auto id = UUTID::new_id();
        result_data[i] = StringVector::AddString(result, id.to_string());
        validity.Set(i, true);
    }
}

// Generate new TSID (no input parameter)
static void TsidScalarFunNoArgs(DataChunk &args, ExpressionState &state, Vector &result) {
    auto result_data = FlatVector::GetData<string_t>(result);
    auto &validity = FlatVector::Validity(result);
    
    for (idx_t i = 0; i < args.size(); i++) {
        auto id = UUTID::new_id();
        result_data[i] = StringVector::AddString(result, id.to_string());
        validity.Set(i, true);
    }
}

// Extract timestamp from TSID
static void TsidToTimestampScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
    UnaryExecutor::Execute<string_t, timestamp_t>(
        args.data[0], result, args.size(),
        [&](string_t input) {
            try {
                auto id = UUTID::from_string(input.GetString());
                auto tp = id.time();
                auto micros = std::chrono::duration_cast<std::chrono::microseconds>(
                    tp.time_since_epoch()).count();
                return timestamp_t(micros);
            } catch (const std::exception &e) {
                throw ConversionException("Invalid TSID format: %s", input.GetString());
            }
        });
}

static void LoadInternal(DatabaseInstance &instance) {
    // Register tsid() functions
    ScalarFunctionSet set("tsid");
    
    // Add variant with text parameter
    ScalarFunction tsid_fun({LogicalType::VARCHAR}, LogicalType::VARCHAR, TsidScalarFun);
    tsid_fun.stability = FunctionStability::VOLATILE;
    set.AddFunction(tsid_fun);
    
    // Add variant without parameters
    ScalarFunction tsid_fun_no_args({}, LogicalType::VARCHAR, TsidScalarFunNoArgs);
    tsid_fun_no_args.stability = FunctionStability::VOLATILE;
    set.AddFunction(tsid_fun_no_args);
    
    ExtensionUtil::RegisterFunction(instance, set);

    // Register tsid_to_timestamp() function
    auto tsid_to_timestamp_function = ScalarFunction(
        "tsid_to_timestamp", {LogicalType::VARCHAR}, LogicalType::TIMESTAMP,
        TsidToTimestampScalarFun
    );
    ExtensionUtil::RegisterFunction(instance, tsid_to_timestamp_function);
}

void TsidExtension::Load(DuckDB &db) {
    LoadInternal(*db.instance);
}

std::string TsidExtension::Name() {
    return "tsid";
}

std::string TsidExtension::Version() const {
#ifdef EXT_VERSION_TSID
    return EXT_VERSION_TSID;
#else
    return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void tsid_init(duckdb::DatabaseInstance &db) {
    duckdb::DuckDB db_wrapper(db);
    db_wrapper.LoadExtension<duckdb::TsidExtension>();
}

DUCKDB_EXTENSION_API const char *tsid_version() {
    return duckdb::DuckDB::LibraryVersion();
}

}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
