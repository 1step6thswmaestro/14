#include "fat.h"

/*
   함수명  : readFATIndex
   하는일  : FATIndexSize를 구하기 위해 SuperBlock을 읽는다.
   계산을 통해 Volume에서 읽을 위치를 구한다.
   그 위치를 읽어 FAT Index값을 리턴해준다.
   인자    : fVolume : 루프백이미지/볼륨의 파일 포인터
nIdx    : FAT의 인덱스
리턴 값 : u128
 */

u32_t read_fat_index(struct mfs_volume* volume, u128 index)
{
	struct mfs_sb_info sb;
	read_sb(volume, &sb);

	u32_t start_position = 0;
	u32_t end_position = 0;
	u128 read_position = 0;
	u32_t value = 0;

	start_position = (sb.fat_sector * sb.bytes_per_sector);
	end_position = (sb.fat_sector + (sb.sectors_of_fat * sb.copies_of_fat)) * sb.bytes_per_sector;

	// Volume에서 읽을 위치를 계산한다. FAT Begin Address + FAT Index Address(Index - 2는 Index 0,1은 쓰이지 않기 때문이다. 시작이 2부터이다.)
	read_position = start_position + (sb.fat_index_size * (index - 2));

	if(read_position > end_position)
		return FALSE;

#ifdef __KERNEL__
	seek_volume(volume, read_position);
#else
	seek_volume(volume, read_position, SEEK_SET);
#endif
	read_volume(volume, &value, sizeof(u8_t), sb.fat_index_size);

	return value;
}

/*
   함수명  : writeFATIndex
   하는일  : FAT안에 nIdx 위치에 nValue 값을 write한다.
   인자    : fVolume : 루프백이미지/파일 볼륨의 포인터
nIdx : FAT Index 번호
nValue : FAT Index에 write할 값
리턴 값 : BOOL
 */

BOOL write_in_fat_index(struct mfs_volume* volume, u32_t index, u64_t value)
{
	struct mfs_sb_info sb;
	read_sb(volume, &sb);

	u32_t start_position = 0;
	u32_t end_position = 0;
	u128 write_position = 0;

	start_position = (sb.fat_sector * sb.bytes_per_sector);
	end_position = (sb.fat_sector + (sb.sectors_of_fat * sb.copies_of_fat)) * sb.bytes_per_sector;

	// Volume에 적을 위치를 계산한다. FAT Begin Address + FAT Index Address(Index - 2는 Index 0,1은 쓰이지 않기 때문이다. 시작이 2부터이다.)
	write_position = start_position + (sb.fat_index_size * (index - 2));

	if(write_position > end_position)
		return FALSE;

#ifdef __KERNEL__
	seek_volume(volume, write_position);
#else
	seek_volume(volume, write_position, SEEK_SET);
#endif
	write_volume(volume, &value, sizeof(sb.fat_index_size), 1);

	return TRUE;
}

/*
   함수명  : search_emptyFATIndex
   하는일  : FAT안에 비어있는 인덱스를 찾는다.
   FAT 시작 주소부터 FAT 끝 주소까지, FATIndexSize 만큼 0를 검색해 나간다.
   인자    : fVolume : 루프백이미지/파일 볼륨의 포인터
   리턴 값 : u32
 */

u32_t find_empty_fat_index(struct mfs_volume* volume)
{
	u8_t sector[BYTES_PER_SECTOR] = {0, };
	u32_t start_position = 0;
	u32_t end_position = 0;
	u32_t read_position = 0;
	u32_t fat_index_value = 0;
	u32_t index = 0;

	struct mfs_sb_info sb;
	read_sb(volume, &sb);
	u32_t fat_index_size = sb.fat_index_size;

	start_position = (sb.fat_sector * sb.bytes_per_sector);
	end_position = (sb.fat_sector + (sb.sectors_of_fat * sb.copies_of_fat)) * sb.bytes_per_sector;

	for(read_position = start_position; read_position < end_position; read_position += BYTES_PER_SECTOR)
	{
#ifdef __KERNEL__
		seek_volume(volume, read_position);
#else
		seek_volume(volume, read_position, SEEK_SET);
#endif
		read_volume(volume, &sector, sizeof(u8_t), sizeof(sector));

		for(index = 0; index < BYTES_PER_SECTOR; index += fat_index_size)
		{
			fat_index_value = *((u32_t *)(sector + index));

			if (fat_index_value == 0) {
				return (((read_position - start_position) / fat_index_size) + (index / fat_index_size) + 2);
			}
		}
	}

	// FAT안에 비어있는 Index가 존재하지 않는다.
	return FALSE;
}


