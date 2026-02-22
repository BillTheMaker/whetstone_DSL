// Step 812: Ambiguity packet model (10 tests)
#include "legacy_ingestion/AmbiguityPacket.h"
#include <iostream>
static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}
void t1(){T(todo_conflict);auto x=AmbiguityPacketModel::build("TODO");C(x.kind==AmbiguityKind::conflict,"kind");P();}
void t2(){T(guess_assumed);auto x=AmbiguityPacketModel::build("guess");C(x.kind==AmbiguityKind::assumed,"kind");P();}
void t3(){T(generic_unknown);auto x=AmbiguityPacketModel::build("other");C(x.kind==AmbiguityKind::unknown,"kind");P();}
void t4(){T(json_shape);auto j=AmbiguityPacketModel::toJson(AmbiguityPacketModel::build("todo"));C(j.contains("kind")&&j.contains("id"),"shape");P();}
void t5(){T(machine_readable);auto j=AmbiguityPacketModel::toJson(AmbiguityPacketModel::build("todo"));C(j.is_object(),"obj");P();}
void t6(){T(deterministic);auto a=AmbiguityPacketModel::toJson(AmbiguityPacketModel::build("todo")).dump();auto b=AmbiguityPacketModel::toJson(AmbiguityPacketModel::build("todo")).dump();C(a==b,"det");P();}
void t7(){T(note_present);auto x=AmbiguityPacketModel::build("todo");C(!x.note.empty(),"note");P();}
void t8(){T(id_set);auto x=AmbiguityPacketModel::build("todo");C(!x.id.empty(),"id");P();}
void t9(){T(conflict_note);auto x=AmbiguityPacketModel::build("TODO");C(x.note.find("TODO")!=std::string::npos,"note");P();}
void t10(){T(assumed_note);auto x=AmbiguityPacketModel::build("guess");C(x.note.find("heuristic")!=std::string::npos,"note");P();}
int main(){std::cout<<"Step 812: AmbiguityPacket\n";t1();t2();t3();t4();t5();t6();t7();t8();t9();t10();std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";return f?1:0;}
