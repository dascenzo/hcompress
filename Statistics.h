#pragma once
#include <filesystem>

struct FileSizeChange {
  using size_type = decltype(file_size(std::declval<std::filesystem::path>()));
  size_type originalSize,
            finalSize;
};
struct SpaceSaving {
  SpaceSaving(FileSizeChange::size_type compressedSize,
              FileSizeChange::size_type decompressedSize);
  union {
    FileSizeChange::size_type emptyFileCompressedSize;
    double percentSaving;
  };
  enum { EMPTY, PERCENT } type;
};
struct Statistics {
  FileSizeChange sizeChange;
  SpaceSaving spaceSaving;
};
