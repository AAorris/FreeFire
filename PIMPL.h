#undef CTOR
#undef DTOR
#undef IMPL

#ifdef CLASS
#define CTOR CLASS::CLASS
#define DTOR CLASS::~CLASS
#define IMPL CLASS::Impl
#endif