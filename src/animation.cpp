#include "pch.h"
#include "animation.h"

void Anime::Read(std::iostream &ss)
{
    ss.seekg(0, ios::end);
    size_t packSize = ss.tellg();
    ss.seekg(0, ios::beg);

    std::unique_ptr<uint8_t[]> packMemory = std::make_unique<uint8_t[]>(packSize);
    uint8_t* packData = packMemory.get();

    ss.read((char*)packData, packSize);

    AnimePackHeader* packHeader = (AnimePackHeader*)packData;
    AnimePackEntry* packEntries = (AnimePackEntry*)(packHeader + 1);

    for (uint32_t entryIdx = 0; entryIdx != packHeader->entryCount; ++entryIdx)
    {
        AnimePackEntry& packEntry = packEntries[entryIdx];

        AnimeGroupHeader* groupHeader = (AnimeGroupHeader*)(packData + packEntry.offset);
        AnimeGroupEntry* groupEntries = (AnimeGroupEntry*)(groupHeader + 1);
        cout << "\t" << "GroupName: " << groupHeader->name << "\n";
        cout << "\t" << "GroupType: " << groupHeader->type << "\n";

        if (groupHeader->type == 5)  // not sure this type
        {
            cout << "\t" << "Not Supported: " << groupHeader->name << "\n";
            continue;
        }

        for (uint32_t groupIdx = 0; groupIdx != groupHeader->entryCount; ++groupIdx)
        {
            AnimeGroupEntry& groupEntry = groupEntries[groupIdx];

            AnimeDefHeader* defHeader = (AnimeDefHeader*)((uint8_t*)groupHeader + groupEntry.offset);

            cout << "\t\t" << "DefName: " << defHeader->name << "\n";

            if (std::string(defHeader->name) == "navcombatidle_right#navcombatidle_right")
            {
                //__debugbreak();
            }

            AnimeActionHeader* actionHeader = getAnimeAction(defHeader);
            
            extractAction(actionHeader);
        }
    }

}

// 1400EA2D0
AnimeOffsetBlock* Anime::getOffsetBlock(AnimeDefHeader* defHeader, uint32_t index)
{
    AnimeDefHeader* header = defHeader;
    AnimeDefEntry* entries = (AnimeDefEntry*)(header + 1);

    for (header = defHeader;
        (header->flags & 0x8000) != 0;
        entries = (AnimeDefEntry*)(header + 1),
        header = (AnimeDefHeader*)((uint8_t*)header + entries[0].size))
    {
    }

    if ((header->flags & 0x8000) == 0)
    {
        return (AnimeOffsetBlock*)((uint8_t*)header + entries[index].offset);
    }

    // step to next header and find again
    AnimeDefHeader* nextDefHeader = (AnimeDefHeader*)((uint8_t*)defHeader + entries[0].size);
    AnimeDefEntry* entry = getAnimeDefEntry(nextDefHeader, 0);
    return (AnimeOffsetBlock*)((uint8_t*)header + entry[index].offset);
}

AnimeDefEntry* Anime::getAnimeDefEntry(AnimeDefHeader* defHeader, uint32_t index)
{
    AnimeDefHeader* header = defHeader;
    AnimeDefEntry* entries = (AnimeDefEntry*)(header + 1);

    for (header = defHeader;
        (header->flags & 0x8000) != 0; 
        entries = (AnimeDefEntry*)(header + 1),
        header = (AnimeDefHeader*)((uint8_t*)header + entries[0].size))
    {
    }

    return &entries[index];
}

// 14015DB10
AnimeActionHeader* Anime::getAnimeAction(AnimeDefHeader* defHeader)
{
    AnimeActionHeader* result = nullptr;
    do 
    {
        if (!defHeader)
        {
            break;
        }

        AnimeDefEntry* entry = (AnimeDefEntry*)(defHeader + 1);
        if ((defHeader->flags & 0x8000) != 0)
        {
            AnimeDefHeader* nextHeader = (AnimeDefHeader*)((uint8_t*)defHeader + entry->size);
            entry = getAnimeDefEntry(nextHeader, 0);
        }

        if (!entry->valid)
        {
            break;
        }

        AnimeOffsetBlock* block = getOffsetBlock(defHeader, 0);
        size_t actionOffset = block->offset << 0x10 +  ((block->shift & 0xC000) << 2) | block->mask ;

        result = (AnimeActionHeader*)((uint8_t*)block + actionOffset);
    } while (false);
    return result;
}

void Anime::extractAction(AnimeActionHeader* action)
{
    do 
    {
        if (!action)
        {
            cout << "Empty Action" << "\n";
            break;
        }

        auto getDispatchEntrySize = [](AnimeDispatchEntry* entry) 
        {
            return (entry->wordCountMinusOne + 1) * sizeof(uint16_t);
        };

        AnimeActionEntry* actionEntries = (AnimeActionEntry*)((uint8_t*)action + action->entryOffset);
        for (uint16_t i = 0; i != action->entryCount; ++i)
        {
            AnimeActionEntry& actionEntry = actionEntries[i];

            uint8_t* base = (uint8_t*)action + actionEntry.dispatchTableOffset;

            AnimeDispatchEntry* dispatchEntry = (AnimeDispatchEntry*)base;
            size_t entrySize = getDispatchEntrySize(dispatchEntry);
            while (true)
            {
                uint8_t dispatchId = dispatchEntry->dispatchId;
                if (dispatchId == 0)
                {
                    break;
                }

                if (dispatchId >= 0x80)
                {
                    // actually the game has another table in case dispatch
                    // id is greater than or equal to 0x80, but I never see
                    // it is used.
                    // 0000000140B320BB
                    break;
                }

                dispatchAnimeData(base, dispatchEntry);

                dispatchEntry = (AnimeDispatchEntry*)((uint8_t*)dispatchEntry + entrySize);
                entrySize = (dispatchEntry->wordCountMinusOne + 1) * sizeof(uint16_t);
            }
        }
    } while (false);
}

void Anime::dispatchAnimeData(uint8_t* base, AnimeDispatchEntry* table)
{
    AnimeDataType type = static_cast<AnimeDataType>(table->dispatchId);
    uint16_t* dataTable = (uint16_t*)((uint8_t*)table + sizeof(AnimeDispatchEntry));

    cout << "\t\t\t" << "DataType (DispatchId): " << (int)type << "\n";

    switch (type)
    {
    case AnimeDataType::Pose:
        processPoseData(base, (AnimePoseInfo*)dataTable);
        break;
    default:
        break;
    }
}

// 0000000140B315C0
void Anime::processPoseData(uint8_t* base, AnimePoseInfo* info)
{
    cout << "\t\t\t\t" << "PoseTable: " << static_cast<void*>(info) << "\n";

    if (info->dataOffset & 3 != 0)
    {
        cout << "**************** Not Supported ****************" << "\n";
    }

    size_t offset = info->dataOffset & 0xFFFFFFFFFFFFFFFC;
    void* poseData = base + offset;

    cout << "\t\t\t\t" << "Info w1: " << (int)info->selector << "  ";
    cout << "Info w2: " << (int)info->dataOffset << "  ";
    cout << "Info w3: " << (int)info->otherOffset << "\n";

    cout << "\t\t\t\t" << "PoseData: " << poseData << "\n";

    decodePoseData((__m128i*)poseData, info->lineCount);
}

void Anime::decodePoseData(__m128i* data, uint16_t lineCount)
{
    __m128i exp_i = _mm_set_epi32(0, 0, 0, 0x38000000);
    __m128 exp_f = _mm_castsi128_ps(exp_i);
    __m128 factor = _mm_shuffle_ps(exp_f, exp_f, 0);

    for (uint16_t i = 0; i != lineCount; ++i)
    {
        __m128i* encodeData = data + i;
        __m128i value = _mm_loadu_si128(encodeData);

        __m128i value_r = _mm_srai_epi32(value, 0x10);
        __m128 value_rf = _mm_cvtepi32_ps(value_r);
        value_rf = _mm_mul_ps(value_rf, factor);

        __m128i value_l = _mm_slli_epi32(value, 0x10);
        value_l = _mm_srai_epi32(value_l, 0x10);
        __m128 value_lf = _mm_cvtepi32_ps(value_l);
        value_lf = _mm_mul_ps(value_lf, factor);

        alignas(16) float result_r[4] = {0};
        alignas(16) float result_l[4] = {0};
        _mm_store_ps(result_r, value_rf);
        _mm_store_ps(result_l, value_lf);

        cout << "\t\t\t\t" << "PoseSample: " << result_r[0] << " " << result_r[1] << " " << result_r[2] << " " << result_r[3] << "\n";
        cout << "\t\t\t\t" << "PoseSample: " << result_l[0] << " " << result_l[1] << " " << result_l[2] << " " << result_l[3] << "\n";
    }
    
}

