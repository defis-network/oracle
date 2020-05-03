project(oracle)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( oracle oracle oracle.cpp )
target_include_directories( oracle PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( oracle ${CMAKE_SOURCE_DIR}/../ricardian )