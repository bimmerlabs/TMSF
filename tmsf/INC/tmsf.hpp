#pragma once

#include <srl.hpp>
#include "rle.hpp"

/** @brief Detect whether object has size function
 * @tparam T Object type
 */
template<typename T>
concept HasLoadSizeFunction = requires {
    { std::declval<T>().LoadSize() } -> std::same_as<size_t>;
};

/** @brief Get object pointer from stream buffer
 * @tparam T Object type
 * @param iterator Stream buffer
 * @param count Number of objects
 * @return T* Object pointer
 */
template<typename T>
T* GetAndIterate(char*& iterator, size_t count = 1)
{
    T* ptr = reinterpret_cast<T*>(iterator);

    if constexpr (HasLoadSizeFunction<T>)
    {
        iterator += ptr->LoadSize() * count;
    }
    else
    {
        iterator += (sizeof(T) * count);
    }

    return ptr;
}
    
/** @brief TileMap object
 */
class TilemapObject
{
private:
    char* fileBuffer = nullptr;   // store as member
    bool ownsBuffer = false;      // track if destructor should free it

    /** @brief Tile Map Sprite Format file header
     */
    struct TmsfHeader
    {
        /** @brief Number of tilemaps stored in the file (not the number of sprites!) 
         */
        uint16_t NumTilesets;

        /** @brief Palette Mode, (SGL tyles)
         */
        uint16_t PaletteMode; // put in image struct instead?

        /** @brief Compression on/off (future)
         */
        uint16_t Compression;
        
        /** @brief Format version string
         */
        char Version[4];
    };

    /** @brief sprite sheet header
     */
    struct SpriteSheetHeader
    {        
        /** @brief Width of the Sprite
         */
        uint16_t CellWidth;

        /** @brief Height of the Sprite
         */
        uint16_t CellHeight;

        /** @brief Columns in sprite sheet
         */
        uint16_t Cols;
                
        /** @brief Rows in sprite sheet
         */
        uint16_t Rows;
        
        /** @brief Number of frames per sprite (not the total number of sprites)
         */
        uint16_t Frames;
        
        // only needed if tilemap size is bigger than 65536 bytes
        // /** @brief Tilemap size (upper 2 bytes)
         // */
        // uint16_t TotalBytesUpper;
        
        // /** @brief Tilemap size (lower 2 bytes)
         // */
        // uint16_t TotalBytesLower;
        
        /** @brief Tilemap size (in bytes, compressed or uncompressed)
         */
        uint16_t TotalBytes;

        /** @brief Total Number of Sprites
         * @return Total number of Sprites inside the spritesheet
         */
        uint16_t SpriteCount() const
        {
            return (Rows * Cols);
        }

        /** @brief Object size
         * @return Object size
         */
        size_t LoadSize() const
        {
            // Remove SRL::Types::HighColor multiplier (my format is only 1 byte per pixel, may need to adjust for other color modes)
            // return sizeof(SpriteSheetHeader) + ((CellWidth * CellHeight) * (Rows * Cols));
            return sizeof(SpriteSheetHeader) + TotalBytes;
        }
        
        /** @brief Current Cell of the sprite sheet
         * @return Current Cell
         */
        size_t CurrentCell(uint16_t index) const
        {
            // Remove SRL::Types::HighColor multiplier (my format is only 1 byte per pixel, may need to adjust for other color modes)
            return (index * (CellWidth * CellHeight));
        }

        /** @brief Object data
         * @return The data pointer
         */
        // currently on supports 8bpp
        SRL::Types::HighColor* Data(uint16_t index) const
        {
            // because it gets expanded to 2 bytes, the headers have to be 2 bytes too?
            return (SRL::Types::HighColor*)(((char*)this) + sizeof(SpriteSheetHeader) + CurrentCell(index));
        }
    };
    
public:
    /** @brief sprite set
     */
    struct SpriteSet
    {        
        /** @brief Pointer to the first Sprite Cell
         */
        int32_t SpriteIndex;
        
        /** @brief Width of the Sprite
         */
        uint16_t CellWidth;

        /** @brief Height of the Sprite
         */
        uint16_t CellHeight;

        /** @brief Columns in sprite sheet
         */
        uint16_t Cols;
        
        /** @brief Rows in sprite sheet
         */
        uint16_t Rows;
        
        /** @brief Number of frames per sprite (not the total number of sprites)
         */
        uint16_t MaxFrames;
        
        /** @brief Current frame
         */
        uint32_t Frame;

        /** @brief Size of each tile
         */
        uint16_t SpriteSize() const
        {
            return (CellWidth * CellHeight);
        }

        /** @brief Total Number of Sprites
         * @return Total number of Sprites inside the spritesheet
         */
        uint16_t SpriteCount() const
        {
            return (Rows * Cols);
        }
    };
    
    SpriteSet * sprite;
    
    /** @brief Asset Pack
     */
    struct AssetPack
    {      
        /** @brief Number of tilemaps (not the number of sprites!) 
         */
        uint16_t NumTilesets;

        /** @brief Palette Mode, (uses SRL::CRAM::TextureColorMode definitions)
         */
        uint16_t PaletteMode;
    };
    
    AssetPack * asset;
    
    /** @brief Initializes a new tilemap sprite sheet object from a file
     * @param tilemapFile Tilemap Sprite file
     * @return AssetPack
     * @return SpriteSets
     */
    TilemapObject(const char* tilemapFile, uint8_t paletteIndex, bool keepBuffer = false)
    {
        SRL::Cd::File file = SRL::Cd::File(tilemapFile);

        // char* fileBuffer = new char[file.Size.Bytes];
        fileBuffer = new char[file.Size.Bytes];   // store in member
        file.LoadBytes(0, file.Size.Bytes, fileBuffer);

        char* iterator = fileBuffer;
        
        TmsfHeader* header = GetAndIterate<TmsfHeader>(iterator);
        
        asset = new AssetPack;
        asset->NumTilesets = header->NumTilesets;
        asset->PaletteMode = header->PaletteMode;
        sprite = new SpriteSet[header->NumTilesets];
        
        // Load textures
        for (size_t i = 0; i < header->NumTilesets; i++)
        {
            // Get sprite header
            SpriteSheetHeader* spritesheet = GetAndIterate<SpriteSheetHeader>(iterator);            
            // for (size_t j = 0; j < spritesheet->SpriteCount(); j++)
            // {
                // // Get texture data
                // int32_t spriteIndex = SRL::VDP1::TryLoadTexture(spritesheet->CellWidth, spritesheet->CellHeight, (SRL::CRAM::TextureColorMode)header->PaletteMode, paletteIndex, spritesheet->Data((uint16_t)j));
                
            const uint8_t* srcData = reinterpret_cast<const uint8_t*>(
                ((char*)spritesheet) + sizeof(SpriteSheetHeader)
            );
            const size_t pixelCount = spritesheet->CellWidth * spritesheet->CellHeight * spritesheet->SpriteCount();

            const uint8_t* inputData = srcData;
            uint8_t* decompressed = nullptr;

            if (header->Compression == 1)
            {
                // decompress once for entire sheet
                decompressed = new uint8_t[pixelCount];
                RLE_Decompress(inputData, decompressed, pixelCount);
                inputData = decompressed;
            }
                 
            for (size_t j = 0; j < spritesheet->SpriteCount(); j++)
            {
                const uint8_t* frameData = inputData + (j * spritesheet->CellWidth * spritesheet->CellHeight);

                int32_t spriteIndex = SRL::VDP1::TryLoadTexture(
                    spritesheet->CellWidth,
                    spritesheet->CellHeight,
                    (SRL::CRAM::TextureColorMode)header->PaletteMode,
                    paletteIndex,
                    const_cast<uint8_t*>(frameData)
                );

                if (spriteIndex < 0)
                {
                    SRL::Debug::Assert("Warning: No free texture space %3d, %3d", i, j);
                }
                else if (j == 0)
                {
                    sprite[i].CellWidth = spritesheet->CellWidth;
                    sprite[i].CellHeight = spritesheet->CellHeight;
                    sprite[i].Rows = spritesheet->Rows;
                    sprite[i].Cols = spritesheet->Cols;
                    sprite[i].MaxFrames = spritesheet->Frames;
                    sprite[i].SpriteIndex = spriteIndex;
                    sprite[i].Frame = spriteIndex;
                }
            }
            
        if (header->Compression == 1)
            delete[] decompressed;
        }
        
        if (!keepBuffer)
        {
            delete[] fileBuffer;  // correct matching delete[]
            fileBuffer = nullptr;
        }
        else
        {
            ownsBuffer = true;    // destructor will free it
        }
    }

    /** @brief Destroy the Tilemap object and free its resources
     */
     // untested
    ~TilemapObject()
    {
        if (ownsBuffer && fileBuffer) {
            delete[] fileBuffer;
        }
        delete[] sprite;
        delete asset;
    }
};