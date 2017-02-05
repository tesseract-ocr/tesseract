#include <stdio.h>
#include <stdint.h>
#include "tessio.h"
#include "fontinfo.h"
#include "../classify/intproto.h"

bool fread(uint8_t* data, FILE* f, size_t n) {
  return fread(data, sizeof(*data), n, f) == n;
}

bool fread(uint16_t* data, FILE* f, size_t n) {
  size_t m = fread(data, sizeof(*data), n, f);
  for (size_t i = 0; i < m; i++)
    convert2le(data[i]);
  return n == m;
}

bool fread(uint32_t* data, FILE* f, size_t n) {
  size_t m = fread(data, sizeof(*data), n, f);
  for (size_t i = 0; i < m; i++)
    convert2le(data[i]);
  return n == m;
}

bool fread(uint64_t* data, FILE* f, size_t n) {
  size_t m = fread(data, sizeof(*data), n, f);
  for (size_t i = 0; i < m; i++)
    convert2le(data[i]);
  return n == m;
}

bool fread(int8_t* data, FILE* f, size_t n) {
  return fread((uint8_t *)data, f, n);
}

bool fread(int16_t* data, FILE* f, size_t n) {
  return fread((uint16_t *)data, f, n);
}

bool fread(int32_t* data, FILE* f, size_t n) {
  return fread((uint32_t *)data, f, n);
}

bool fread(int64_t* data, FILE* f, size_t n) {
  return fread((uint64_t *)data, f, n) ;
}

bool fread(char* data, FILE* f, size_t n) {
  return fread(data, sizeof(*data), n, f) == n;
}

bool fread(float* data, FILE* f, size_t n) {
  assert(0);
  return fread(data, sizeof(*data), n, f) == n;
}

namespace tesseract {
bool fread(FontInfo* data, FILE* f, size_t n)
{
	return fread(data, sizeof(*data), n, f);
}

bool fread(FontSet* data, FILE* f, size_t n)
{
	return fread(data, sizeof(*data), n, f);
}
} // namespace tesseract

bool fread(CLASS_PRUNER_STRUCT* data, FILE* f, size_t n)
{
	return fread(&data->p[0][0][0][0], f, n * 24 * 24 * 24 * 2);
}

bool fread(INT_FEATURE_STRUCT* data, FILE* f, size_t n)
{
	return fread(data, sizeof(*data), n, f);
}

//~ bool fread(INT_PROTO_STRUCT* data, FILE* f, size_t n = 1);

bool fread(PROTO_SET_STRUCT* data, FILE* f, size_t n)
{
	//~ typedef uinT32 PROTO_PRUNER[NUM_PP_PARAMS][NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR];
	//~ PROTO_PRUNER ProtoPruner;
	//~ INT_PROTO_STRUCT Protos[PROTOS_PER_PROTO_SET];
	//~ struct INT_PROTO_STRUCT {
	  //~ inT8 A;
	  //~ uinT8 B;
	  //~ inT8 C;
	  //~ uinT8 Angle;
	  //~ uinT32 Configs[WERDS_PER_CONFIG_VEC];
	//~ };
	assert(n == 1);
	fread(&data->ProtoPruner[0][0][0], f, NUM_PP_PARAMS * NUM_PP_BUCKETS * WERDS_PER_PP_VECTOR);
	for (unsigned i = 0; i < PROTOS_PER_PROTO_SET; i++) {
		fread(&data->Protos[i].A, f, 4);
		fread(&data->Protos[i].Configs[0], f, WERDS_PER_CONFIG_VEC);
	}
	return true;
}
