extern crate duckdb;
extern crate duckdb_loadable_macros;
extern crate libduckdb_sys;
use tsid::create_tsid;

use duckdb::{
    core::{DataChunkHandle, Inserter, LogicalTypeHandle, LogicalTypeId},
    vtab::{BindInfo, Free, FunctionInfo, InitInfo, VTab},
    Connection, Result,
};
use duckdb_loadable_macros::duckdb_entrypoint_c_api;
use libduckdb_sys as ffi;
use std::{
    error::Error,
    ffi::{c_char, CString},
};

// Scalar Function
#[repr(C)]
struct TsidBindData;

#[repr(C)]
struct TsidInitData {
    done: bool,
}

struct TsidVTab;

impl Free for TsidBindData {}
impl Free for TsidInitData {}

impl VTab for TsidVTab {
    type InitData = TsidInitData;
    type BindData = TsidBindData;

    unsafe fn bind(bind: &BindInfo, _: *mut TsidBindData) -> Result<(), Box<dyn std::error::Error>> {
        bind.add_result_column("tsid", LogicalTypeHandle::from(LogicalTypeId::Varchar));
        Ok(())
    }

    unsafe fn init(_: &InitInfo, data: *mut TsidInitData) -> Result<(), Box<dyn std::error::Error>> {
        unsafe {
            (*data).done = false;
        }
        Ok(())
    }

    unsafe fn func(func: &FunctionInfo, output: &mut DataChunkHandle) -> Result<(), Box<dyn std::error::Error>> {
        let init_info = func.get_init_data::<TsidInitData>();
        unsafe {
            if (*init_info).done {
                output.set_len(0);
            } else {
                (*init_info).done = true;
                let vector = output.flat_vector(0);
                let tsid = create_tsid();
                vector.insert(0, CString::new(tsid.to_string())?);
                output.set_len(1);
            }
        }
        Ok(())
    }

    fn parameters() -> Option<Vec<LogicalTypeHandle>> {
        None
    }
}

const EXTENSION_NAME: &str = env!("CARGO_PKG_NAME");

#[duckdb_entrypoint_c_api(ext_name = "tsid", min_duckdb_version = "v0.0.1")]
pub unsafe fn extension_entrypoint(con: Connection) -> Result<(), Box<dyn Error>> {
    con.register_table_function::<TsidVTab>("tsid")
        .expect("Failed to register tsid table function");
    Ok(())
}
