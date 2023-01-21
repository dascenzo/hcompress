#include "Statistics.h"

SpaceSaving::SpaceSaving(FileSizeChange::size_type compressedSize, FileSizeChange::size_type decompressedSize) {
  if (decompressedSize == 0) {
    type = SpaceSaving::EMPTY;
    emptyFileCompressedSize = compressedSize;
  }
  else {
    type = SpaceSaving::PERCENT;
    percentSaving = 
        (1.0 - (static_cast<double>(compressedSize) / decompressedSize)) * 100;
  }
}
