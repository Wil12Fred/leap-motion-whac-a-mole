// Unit tests for the separable helpers shared by the client and the server:
// fillZeros and the Base64 file encoder/decoder in utilidad.h, and the OBJ
// loader in Model/ParseObj/obj.h that the client uses for every model.
// None of them needs OpenGL or the Leap Motion SDK.
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>
#include "utilidad.h"
#include "vector2.h"
#include "vector3.h"
#include "vertex.h"
#include "material.h"
#include "mesh.h"
#include "ParseObj/obj.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); ++failures; } } while (0)

static void test_fill_zeros() {
  CHECK(fillZeros(7, 4) == "0007");
  CHECK(fillZeros(123, 2) == "123");    // never truncates
  CHECK(fillZeros(0, 3) == "000");
}

static std::string write_file(const std::string& name, const std::string& bytes) {
  std::ofstream f(name.c_str(), std::ios::binary);
  f << bytes;
  return name;
}

static std::string read_file(const std::string& name) {
  std::ifstream f(name.c_str(), std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

// Encoding against RFC 4648 test vectors, including the two padded tails.
static void test_base64_known_vectors() {
  const char* in[]  = {"", "f", "fo", "foo", "foob", "fooba", "foobar", "Hello"};
  const char* out[] = {"", "Zg==", "Zm8=", "Zm9v", "Zm9vYg==", "Zm9vYmE=", "Zm9vYmFy", "SGVsbG8="};
  for (int i = 0; i < 8; i++) {
    std::string name = write_file("b64.in", in[i]), b64;
    CHECK(readFileContentToB64(name, b64));
    if (b64 != out[i]) std::fprintf(stderr, "  encode(\"%s\") = \"%s\", expected \"%s\"\n", in[i], b64.c_str(), out[i]);
    CHECK(b64 == out[i]);
    std::string s(out[i]);
    std::string dec = readB64Content(s);
    if (dec != in[i]) std::fprintf(stderr, "  decode(\"%s\") has %d bytes, expected \"%s\"\n", out[i], (int)dec.size(), in[i]);
    CHECK(dec == in[i]);
  }
}

// File -> Base64 -> file gives the same bytes back, for every length mod 3
// and for bytes with the high bit set.
static void test_base64_round_trip() {
  srand(3);
  for (int len = 0; len < 40; len++) {
    std::string bytes;
    for (int i = 0; i < len; i++) bytes.push_back(char(rand() % 256));
    std::string name = write_file("rt.in", bytes), b64;
    CHECK(readFileContentToB64(name, b64));
    CHECK(b64.size() == (size_t)((len + 2) / 3 * 4));
    std::string outname = "rt.out";
    writeArchiveB64(outname, b64);
    CHECK(read_file(outname) == bytes);
  }
}

static void test_obj_loader_cube() {
  objl::Loader loader;
  CHECK(loader.LoadFile("../data/cube.obj"));
  CHECK(loader.LoadedMeshes.size() == 1);
  CHECK(loader.LoadedIndices.size() == 36);   // six quads -> twelve triangles
  if (loader.LoadedMeshes.empty()) return;
  objl::Mesh& m = loader.LoadedMeshes[0];
  CHECK(m.MeshName == "cube");
  float minx = 1e9, maxx = -1e9;
  for (size_t i = 0; i < m.Vertices.size(); i++) {
    minx = std::min(minx, m.Vertices[i].Position.X);
    maxx = std::max(maxx, m.Vertices[i].Position.X);
  }
  CHECK(minx == 0 && maxx == 1);
  for (size_t i = 0; i < m.Indices.size(); i++) CHECK(m.Indices[i] < m.Vertices.size());
}

int main() {
  test_fill_zeros();
  test_base64_known_vectors();
  test_base64_round_trip();
  test_obj_loader_cube();
  if (failures) { std::printf("%d check(s) failed\n", failures); return 1; }
  std::printf("all utility tests passed\n");
  return 0;
}
