#!/bin/sh

ASSET_DIR="../../../assets/block_textures"
ASS_DIR="assets/block_textures/"
NAME="base_texture_enum"
HEADER="$NAME.h"
SOURCE="$NAME.c"

echo "// auto-generated from $ASSET_DIR the $(date +%Y-%m-%d)" > $HEADER
echo "#pragma once" >> $HEADER
echo "" >> $HEADER

echo -e "typedef enum BaseTextureEnum\n{" >> $HEADER

COUNT=0
for file in "$ASSET_DIR"/*.PNG; do
    base=$(basename "$file" .PNG)
    enum_name="TEX_$(echo "$base" | tr '[:lower:]' '[:upper:]' | sed 's/[^A-Z0-9_]/_/g')"
    echo "    $enum_name," >> $HEADER
    COUNT=$((COUNT + 1))
done

echo "} BaseTextureEnum;" >> $HEADER
echo "" >> $HEADER
echo "#define TEXTURE_COUNT $COUNT" >> $HEADER
echo "" >> $HEADER

echo "extern const char* TexturePaths[TEXTURE_COUNT];" >> $HEADER

echo "// auto-generated source from $ASSET_DIR the $(date +%Y-%m-%d)" > $SOURCE
echo "#include \"$HEADER\"" >> $SOURCE
echo "" >> $SOURCE

echo "// clang-format off" >> $SOURCE
echo "const char* TexturePaths[TEXTURE_COUNT] = {" >> $SOURCE

for file in "$ASSET_DIR"/*.PNG; do
    base=$(basename "$file" .PNG)
    enum_name="TEX_$(echo "$base" | tr '[:lower:]' '[:upper:]' | sed 's/[^A-Z0-9_]/_/g')"
	echo "    [$enum_name] = \"$ASS_DIR$(basename $file)\"," >> $SOURCE
done

echo "};" >> $SOURCE

echo "Generated $HEADER and $SOURCE with $COUNT texture(s)"
