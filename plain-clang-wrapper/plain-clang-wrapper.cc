#include <unistd.h>

#include <cstdio>
#include <string>

int main(int argc, char** argv) {
  freopen("/tmp/plain-clang-wrapper.stdout", "a", stdout);
  freopen("/tmp/plain-clang-wrapper.stderr", "a", stderr);
  fprintf(stdout, "%s\n", std::string(80, '-').c_str());
  for (int i = 0; i < argc; ++i) fprintf(stdout, "%s ", argv[i]);
  fprintf(stdout, "\n");
  fflush(stdout);
  fprintf(stderr, "%s\n", std::string(80, '-').c_str());
  for (int i = 0; i < argc; ++i) fprintf(stderr, "%s ", argv[i]);
  fprintf(stderr, "\n");
  fflush(stderr);
  argv[0] = "clang-13";
  execvp(argv[0], argv);
}
