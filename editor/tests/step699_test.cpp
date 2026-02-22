// Step 699: Rust ownership/borrow graph extractor (12 tests)

#include "rust_ir/RustOwnershipBorrowExtractor.h"

#include <iostream>

static int p = 0, f = 0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if (!(c)) { F(m); return; }

static const char* sample =
    "let owner = data;\n"
    "let borrow = &owner;\n"
    "let borrow_mut = &mut owner;\n"
    "drop(owner);\n";

void t1(){T(extract_constructable);auto g=RustOwnershipBorrowExtractor::extract(sample);C(g.regions.size()>=1,"no regions");P();}
void t2(){T(owned_region_present);auto g=RustOwnershipBorrowExtractor::extract(sample);bool ok=false;for(auto&r:g.regions)if(r.owner=="owner")ok=true;C(ok,"owner region missing");P();}
void t3(){T(shared_borrow_edge_present);auto g=RustOwnershipBorrowExtractor::extract(sample);bool ok=false;for(auto&e:g.edges)if(e.relation=="borrows")ok=true;C(ok,"borrow edge missing");P();}
void t4(){T(mut_borrow_edge_present);auto g=RustOwnershipBorrowExtractor::extract(sample);bool ok=false;for(auto&e:g.edges)if(e.relation=="borrows_mut")ok=true;C(ok,"mut borrow edge missing");P();}
void t5(){T(move_edge_present);auto g=RustOwnershipBorrowExtractor::extract(sample);bool ok=false;for(auto&e:g.edges)if(e.relation=="moves")ok=true;C(ok,"move edge missing");P();}
void t6(){T(drop_diagnostic_present);auto g=RustOwnershipBorrowExtractor::extract(sample);C(!g.diagnostics.empty(),"diagnostic missing");P();}
void t7(){T(validation_passes);auto g=RustOwnershipBorrowExtractor::extract(sample);std::string err;C(RustOwnershipBorrowExtractor::validate(g,&err),err.c_str());P();}
void t8(){T(empty_input_valid);auto g=RustOwnershipBorrowExtractor::extract("");std::string err;C(RustOwnershipBorrowExtractor::validate(g,&err),err.c_str());P();}
void t9(){T(deterministic_output);auto a=RustOwnershipBorrowExtractor::toJson(RustOwnershipBorrowExtractor::extract(sample)).dump();auto b=RustOwnershipBorrowExtractor::toJson(RustOwnershipBorrowExtractor::extract(sample)).dump();C(a==b,"nondeterministic");P();}
void t10(){T(json_packet_has_regions);auto j=RustOwnershipBorrowExtractor::toJson(RustOwnershipBorrowExtractor::extract(sample));C(j.contains("regions"),"regions missing");P();}
void t11(){T(json_packet_has_edges);auto j=RustOwnershipBorrowExtractor::toJson(RustOwnershipBorrowExtractor::extract(sample));C(j.contains("edges"),"edges missing");P();}
void t12(){T(json_packet_has_diagnostics);auto j=RustOwnershipBorrowExtractor::toJson(RustOwnershipBorrowExtractor::extract(sample));C(j.contains("diagnostics"),"diagnostics missing");P();}

int main(){std::cout<<"Step 699: Rust ownership/borrow extractor\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();t11();t12();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
