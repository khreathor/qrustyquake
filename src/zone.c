// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// GPLv3 See LICENSE for details.

// ZONE MEMORY ALLOCATION
// There is never any space between memblocks, and there will never be two
// contiguous free memblocks.
// The rover can be left pointing at a non-empty block
// The zone calls are pretty much only used for small strings and structures,
// all big things are allocated on the hunk.

#include "quakedef.h"

#define DYNAMIC_SIZE 256*1024 //johnfitz: was 48k
#define ZONEID 0x1d4a11
#define MINFRAGMENT 64
#define HUNK_SENTINAL 0x1df001ed

typedef struct {
	int sentinal;
	int size; // including sizeof(hunk_t), -1 = not allocated
	char name[8];
} hunk_t;

typedef struct memblock_s {
	int size; // including the header and possibly tiny fragments
	int tag; // a tag of 0 is a free block
	int id; // should be ZONEID
	struct memblock_s *next, *prev;
	int pad; // pad to 64 bit boundary
} memblock_t;

typedef struct {
	int size; // total bytes malloced, including header
	memblock_t blocklist; // start / end cap for linked list
	memblock_t *rover;
} memzone_t;

typedef struct cache_system_s {
	int size; // including this header
	cache_user_t *user;
	char name[16];
	struct cache_system_s *prev, *next;
	struct cache_system_s *lru_prev, *lru_next; // for LRU flushing
} cache_system_t;

memzone_t *mainzone;
byte *hunk_base;
int hunk_size;
int hunk_low_used;
int hunk_high_used;
qboolean hunk_tempactive;
int hunk_tempmark;
cache_system_t cache_head;

cache_system_t *Cache_TryAlloc(int size, qboolean nobottom);
void Cache_FreeLow(int new_low_hunk);
void Cache_FreeHigh(int new_high_hunk);
void R_FreeTextures();

void Z_Free(void *ptr)
{
	if (!ptr)
		Sys_Error("Z_Free: NULL pointer");
	memblock_t *block = (memblock_t *) ((byte *) ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		Sys_Error("Z_Free: freed a pointer without ZONEID");
	if (block->tag == 0)
		Sys_Error("Z_Free: freed a freed pointer");
	block->tag = 0; // mark as free
	memblock_t *other = block->prev;
	if (!other->tag) { // merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		if (block == mainzone->rover)
			mainzone->rover = other;
		block = other;
	}
	other = block->next;
	if (!other->tag) { // merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		if (other == mainzone->rover)
			mainzone->rover = block;
	}
}

void *Z_Malloc(int size)
{
	// Z_CheckHeap(); // DEBUG
	void *buf = Z_TagMalloc(size, 1);
	if (!buf)
		Sys_Error("Z_Malloc: failed on allocation of %i bytes", size);
	Q_memset(buf, 0, size);
	return buf;
}

void *Z_Realloc(void *ptr, int size)
{
	int old_size;
	void *old_ptr;
	memblock_t *block;

	if (!ptr)
		return Z_Malloc(size);

	block = (memblock_t *) ((byte *) ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		Sys_Error("Z_Realloc: realloced a pointer without ZONEID");
	if (block->tag == 0)
		Sys_Error("Z_Realloc: realloced a freed pointer");

	old_size = block->size;
	old_ptr = ptr;

	Z_Free(ptr);
	ptr = Z_TagMalloc(size, 1);
	if (!ptr)
		Sys_Error("Z_Realloc: failed on allocation of %i bytes", size);

	if (ptr != old_ptr)
		memmove(ptr, old_ptr, min(old_size, size));

	return ptr;
}

void *Z_TagMalloc(int size, int tag)
{
	if (!tag)
		Sys_Error("Z_TagMalloc: tried to use a 0 tag");
	// scan through the block list looking for the first free block of sufficient size
	size += sizeof(memblock_t); // account for size of block header
	size += 4; // space for memory trash tester
	size = (size + 7) & ~7; // align to 8-byte boundary
	memblock_t *start, *rover, *new, *base;
	base = rover = mainzone->rover;
	start = base->prev;
	do {
		if (rover == start) // scaned all the way around the list
			return NULL;
		if (rover->tag)
			base = rover = rover->next;
		else
			rover = rover->next;
	} while (base->tag || base->size < size);
	// found a block big enough
	int extra = base->size - size;
	if (extra > MINFRAGMENT) { // there will be a free fragment after the allocated block
		new = (memblock_t *) ((byte *) base + size);
		new->size = extra;
		new->tag = 0; // free block
		new->prev = base;
		new->id = ZONEID;
		new->next = base->next;
		new->next->prev = new;
		base->next = new;
		base->size = size;
	}
	base->tag = tag; // no longer a free block
	mainzone->rover = base->next; // next allocation will start looking here
	base->id = ZONEID;
	// marker for memory trash testing
	*(int *)((byte *) base + base->size - 4) = ZONEID;
	return (void *)((byte *) base + sizeof(memblock_t));
}

void Z_CheckHeap()
{
	for (memblock_t *block=mainzone->blocklist.next;; block = block->next) {
		if (block->next == &mainzone->blocklist)
			break; // all blocks have been hit
		if ((byte *) block + block->size != (byte *) block->next)
			Sys_Error ("Z_CheckHeap: block size does not touch the next block\n");
		if (block->next->prev != block)
			Sys_Error ("Z_CheckHeap: next block doesn't have proper back link\n");
		if (!block->tag && !block->next->tag)
			Sys_Error("Z_CheckHeap: two consecutive free blocks\n");
	}
}


void Hunk_Check()
{ // Run consistancy and sentinal trahing checks
	for (hunk_t *h = (hunk_t *) hunk_base;
			(byte *) h != hunk_base + hunk_low_used;) {
		if (h->sentinal != HUNK_SENTINAL)
			Sys_Error("Hunk_Check: trahsed sentinal");
		if (h->size < (int)sizeof(hunk_t)
				|| h->size + (byte *) h - hunk_base > hunk_size)
			Sys_Error("Hunk_Check: bad size");
		h = (hunk_t *) ((byte *) h + h->size);
	}
}


void Hunk_Print(qboolean all) // If "all" = true, print all allocs. Otherwise,
{ // allocations with the same name will be totaled up before printing.
	char name[9];
	name[8] = 0;
	int count = 0;
	int sum = 0;
	int totalblocks = 0;
	hunk_t *h = (hunk_t *) hunk_base;
	hunk_t *endlow = (hunk_t *) (hunk_base + hunk_low_used);
	hunk_t *starthigh = (hunk_t *) (hunk_base + hunk_size - hunk_high_used);
	hunk_t *endhigh = (hunk_t *) (hunk_base + hunk_size);
	Con_Printf(" :%8i total hunk size\n", hunk_size);
	Con_Printf("-------------------------\n");
	while (1) {
		// skip to the high hunk if done with low hunk
		if (h == endlow) {
			Con_Printf("-------------------------\n");
			Con_Printf(" :%8i REMAINING\n",
					hunk_size - hunk_low_used - hunk_high_used);
			Con_Printf("-------------------------\n");
			h = starthigh;
		}
		// if totally done, break
		if (h == endhigh)
			break;
		// run consistancy checks
		if (h->sentinal != HUNK_SENTINAL)
			Sys_Error("Hunk_Check: trahsed sentinal");
		if (h->size < (int)sizeof(hunk_t)
				|| h->size + (byte *) h - hunk_base > hunk_size)
			Sys_Error("Hunk_Check: bad size");
		hunk_t *next = (hunk_t *) ((byte *) h + h->size);
		count++;
		totalblocks++;
		sum += h->size;
		// print the single block
		memcpy(name, h->name, 8);
		if (all)
			Con_Printf("%8p :%8i %8s\n", h, h->size, name);

		// print the total
		if(next==endlow||next==endhigh||strncmp(h->name,next->name,8)) {
			if (!all)
				Con_Printf(" :%8i %8s (TOTAL)\n", sum, name);
			count = 0;
			sum = 0;
		}
		h = next;
	}
	Con_Printf("-------------------------\n");
	Con_Printf("%8i total blocks\n", totalblocks);
}

void Hunk_Print_f()
{ // johnfitz -- console command to call hunk_print
	Hunk_Print(false);
}

void *Hunk_AllocName(int size, char *name)
{
	if (size < 0)
		Sys_Error("Hunk_Alloc: bad size: %i", size);
	size = sizeof(hunk_t) + ((size + 15) & ~15);
	if (hunk_size - hunk_low_used - hunk_high_used < size)
		Sys_Error("Hunk_Alloc: failed on %i bytes", size);
	hunk_t *h = (hunk_t *) (hunk_base + hunk_low_used);
	hunk_low_used += size;
	Cache_FreeLow(hunk_low_used);
	memset(h, 0, size);
	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	Q_strncpy(h->name, name, 8);
	return (void *)(h + 1);
}

void *Hunk_Alloc(int size)
{
	return Hunk_AllocName(size, "unknown");
}

int Hunk_LowMark()
{
	return hunk_low_used;
}

void Hunk_FreeToLowMark(int mark)
{
	if (mark < 0 || mark > hunk_low_used)
		Sys_Error("Hunk_FreeToLowMark: bad mark %i", mark);
	memset(hunk_base + mark, 0, hunk_low_used - mark);
	hunk_low_used = mark;
}

int Hunk_HighMark()
{
	if (hunk_tempactive) {
		hunk_tempactive = false;
		Hunk_FreeToHighMark(hunk_tempmark);
	}
	return hunk_high_used;
}

void Hunk_FreeToHighMark(int mark)
{
	if (hunk_tempactive) {
		hunk_tempactive = false;
		Hunk_FreeToHighMark(hunk_tempmark);
	}
	if (mark < 0 || mark > hunk_high_used)
		Sys_Error("Hunk_FreeToHighMark: bad mark %i", mark);
	memset(hunk_base + hunk_size - hunk_high_used, 0,
			hunk_high_used - mark);
	hunk_high_used = mark;
}

void *Hunk_HighAllocName(int size, char *name)
{
	if (size < 0)
		Sys_Error("Hunk_HighAllocName: bad size: %i", size);
	if (hunk_tempactive) {
		Hunk_FreeToHighMark(hunk_tempmark);
		hunk_tempactive = false;
	}
	size = sizeof(hunk_t) + ((size + 15) & ~15);
	if (hunk_size - hunk_low_used - hunk_high_used < size) {
		Con_Printf("Hunk_HighAlloc: failed on %i bytes\n", size);
		return NULL;
	}
	hunk_high_used += size;
	Cache_FreeHigh(hunk_high_used);
	hunk_t *h = (hunk_t *) (hunk_base + hunk_size - hunk_high_used);
	memset(h, 0, size);
	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	Q_strncpy(h->name, name, 8);
	return (void *)(h + 1);
}

void *Hunk_TempAlloc(int size)
{ // Return space from the top of the hunk
	size = (size + 15) & ~15;
	if (hunk_tempactive) {
		Hunk_FreeToHighMark(hunk_tempmark);
		hunk_tempactive = false;
	}
	hunk_tempmark = Hunk_HighMark();
	void *buf = Hunk_HighAllocName(size, "temp");
	hunk_tempactive = true;
	return buf;
}

void Cache_Move(cache_system_t *c)
{
	// we are clearing up space at the bottom, so only allocate it late
	cache_system_t *new = Cache_TryAlloc(c->size, true);
	if (new) {
		Q_memcpy(new + 1, c + 1, c->size - sizeof(cache_system_t));
		new->user = c->user;
		Q_memcpy(new->name, c->name, sizeof(new->name));
		Cache_Free(c->user);
		new->user->data = (void *)(new + 1);
	} else {
		Cache_Free(c->user);
	}
}

void Cache_FreeLow(int new_low_hunk)
{ // Throw things out until the hunk can be expanded to the given point
	cache_system_t *c;

	while (1) {
		c = cache_head.next;
		if (c == &cache_head)
			return; // nothing in cache at all
		if ((byte *) c >= hunk_base + new_low_hunk)
			return; // there is space to grow the hunk
		Cache_Move(c); // reclaim the space
	}
}

void Cache_FreeHigh(int new_high_hunk)
{ // Throw things out until the hunk can be expanded to the given point
	cache_system_t *prev = NULL;
	while (1) {
		cache_system_t *c = cache_head.prev;
		if (c == &cache_head)
			return; // nothing in cache at all
		if ((byte*)c + c->size <= hunk_base + hunk_size - new_high_hunk)
			return; // there is space to grow the hunk
		if (c == prev)
			Cache_Free(c->user); // didn't move out of the way
		else {
			Cache_Move(c); // try to move it
			prev = c;
		}
	}
}

void Cache_UnlinkLRU(cache_system_t *cs)
{
	if (!cs->lru_next || !cs->lru_prev)
		Sys_Error("Cache_UnlinkLRU: NULL link");
	cs->lru_next->lru_prev = cs->lru_prev;
	cs->lru_prev->lru_next = cs->lru_next;
	cs->lru_prev = cs->lru_next = NULL;
}

void Cache_MakeLRU(cache_system_t *cs)
{
	if (cs->lru_next || cs->lru_prev)
		Sys_Error("Cache_MakeLRU: active link");
	cache_head.lru_next->lru_prev = cs;
	cs->lru_next = cache_head.lru_next;
	cs->lru_prev = &cache_head;
	cache_head.lru_next = cs;
}

// Looks for a free block of memory between the high and low hunk marks
// Size should already include the header and padding
cache_system_t *Cache_TryAlloc(int size, qboolean nobottom)
{
	cache_system_t *cs, *new;
	// is the cache completely empty?
	if (!nobottom && cache_head.prev == &cache_head) {
		if (hunk_size - hunk_high_used - hunk_low_used < size)
			Sys_Error("Cache_TryAlloc: %i is greater then free hunk"
				 , size);
		new = (cache_system_t *) (hunk_base + hunk_low_used);
		memset(new, 0, sizeof(*new));
		new->size = size;
		cache_head.prev = cache_head.next = new;
		new->prev = new->next = &cache_head;
		Cache_MakeLRU(new);
		return new;
	}
	// search from the bottom up for space
	new = (cache_system_t *) (hunk_base + hunk_low_used);
	cs = cache_head.next;
	do {
		if (!nobottom || cs != cache_head.next) {
			if ((byte *) cs - (byte *) new >= size) { // found space
				memset(new, 0, sizeof(*new));
				new->size = size;
				new->next = cs;
				new->prev = cs->prev;
				cs->prev->next = new;
				cs->prev = new;
				Cache_MakeLRU(new);
				return new;
			}
		}
		new = (cache_system_t *) ((byte *) cs + cs->size);
		cs = cs->next; // continue looking
	} while (cs != &cache_head);
	// try to allocate one at the very end
	if (hunk_base + hunk_size - hunk_high_used - (byte *) new >= size) {
		memset(new, 0, sizeof(*new));
		new->size = size;
		new->next = &cache_head;
		new->prev = cache_head.prev;
		cache_head.prev->next = new;
		cache_head.prev = new;
		Cache_MakeLRU(new);
		return new;
	}
	return NULL; // couldn't allocate
}


void Cache_Flush()
{ // Throw everything out, so new data will be demand cached
	while (cache_head.next != &cache_head)
		Cache_Free(cache_head.next->user); // reclaim the space
}

void Cache_Report()
{
	Con_DPrintf("%4.1f megabyte data cache\n", (hunk_size - hunk_high_used -
					hunk_low_used) / (float)(1024 * 1024));
}

void Cache_Init()
{
	cache_head.next = cache_head.prev = &cache_head;
	cache_head.lru_next = cache_head.lru_prev = &cache_head;
	Cmd_AddCommand("flush", Cache_Flush);
}

void Cache_Free(cache_user_t *c)
{ // Frees the memory and removes it from the LRU list
	if (!c->data)
		Sys_Error("Cache_Free: not allocated");
	cache_system_t *cs = ((cache_system_t *) c->data) - 1;
	cs->prev->next = cs->next;
	cs->next->prev = cs->prev;
	cs->next = cs->prev = NULL;
	c->data = NULL;
	Cache_UnlinkLRU(cs);
}

void *Cache_Check(cache_user_t *c)
{
	if (!c->data)
		return NULL;
	cache_system_t *cs = ((cache_system_t *) c->data) - 1;
	Cache_UnlinkLRU(cs); // move to head of LRU
	Cache_MakeLRU(cs);
	return c->data;
}

void *Cache_Alloc(cache_user_t *c, int size, char *name)
{
	if (c->data)
		Sys_Error("Cache_Alloc: allready allocated");
	if (size <= 0)
		Sys_Error("Cache_Alloc: size %i", size);
	size = (size + sizeof(cache_system_t) + 15) & ~15;
	while (1) { // find memory for it
		cache_system_t *cs = Cache_TryAlloc(size, false);
		if (cs) {
			strncpy(cs->name, name, sizeof(cs->name) - 1);
			c->data = (void *)(cs + 1);
			cs->user = c;
			break;
		}
		// free the least recently used cahedat
		if (cache_head.lru_prev == &cache_head)
			Sys_Error("Cache_Alloc: out of memory"); // not enough memory at all
		Cache_Free(cache_head.lru_prev->user);
	}
	return Cache_Check(c);
}

static void Memory_InitZone(memzone_t *zone, int size)
{ // set the entire zone to one free block
	memblock_t *block;
	zone->blocklist.next = zone->blocklist.prev = block =
		(memblock_t *) ((byte *) zone + sizeof(memzone_t));
	zone->blocklist.tag = 1; // in use block
	zone->blocklist.id = 0;
	zone->blocklist.size = 0;
	zone->rover = block;
	block->prev = block->next = &zone->blocklist;
	block->tag = 0; // free block
	block->id = ZONEID;
	block->size = size - sizeof(memzone_t);
}

void Memory_Init(void *buf, int size)
{
	int zonesize = DYNAMIC_SIZE;
	hunk_base = buf;
	hunk_size = size;
	hunk_low_used = 0;
	hunk_high_used = 0;
	Cache_Init();
	int p = COM_CheckParm("-zone");
	if (p) {
		if (p < com_argc - 1)
			zonesize = Q_atoi(com_argv[p + 1]) * 1024;
		else
			Sys_Error("Memory_Init: you must specify a size in KB after -zone");
	}
	mainzone = Hunk_AllocName(zonesize, "zone");
	Memory_InitZone(mainzone, zonesize);

	Cmd_AddCommand("hunk_print", Hunk_Print_f); //johnfitz
}
