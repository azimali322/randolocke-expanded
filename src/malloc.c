#include "global.h"
#include "malloc.h"
#include "config/randolocke.h"
#if TESTING
#include "test/test.h"
#endif

static void *sHeapStart;
static u32 sHeapSize;

ALIGNED(4) EWRAM_DATA u8 gHeap[HEAP_SIZE] = {0};

void PutMemBlockHeader(void *block, struct MemBlock *prev, struct MemBlock *next, u32 size)
{
    struct MemBlock *header = (struct MemBlock *)block;

    header->allocated = FALSE;
    header->locationHi = 0;
    header->magic = MALLOC_SYSTEM_ID;
    header->size = size;
    header->locationLo = 0;
    header->prev = prev;
    header->next = next;
}

void PutFirstMemBlockHeader(void *block, u32 size)
{
    PutMemBlockHeader(block, (struct MemBlock *)block, (struct MemBlock *)block, size - sizeof(struct MemBlock));
}

static void *AllocInternal(void *heapStart, u32 size, const char *location)
{
    struct MemBlock *pos = (struct MemBlock *)heapStart;
    struct MemBlock *head = pos;
    struct MemBlock *splitBlock;
    u32 foundBlockSize;

    // Alignment
    if (size & 3)
        size = 4 * ((size / 4) + 1);

    for (;;)
    {
        // Loop through the blocks looking for unused block that's big enough.

        if (!pos->allocated)
        {
            foundBlockSize = pos->size;

            if (foundBlockSize >= size)
            {
                if (foundBlockSize - size < 2 * sizeof(struct MemBlock))
                {
                    // The block isn't much bigger than the requested size,
                    // so just use it.
                    pos->allocated = TRUE;
                }
                else
                {
                    // The block is significantly bigger than the requested
                    // size, so split the rest into a separate block.
                    foundBlockSize -= sizeof(struct MemBlock);
                    foundBlockSize -= size;

                    splitBlock = (struct MemBlock *)(pos->data + size);

                    pos->allocated = TRUE;
                    pos->size = size;

                    PutMemBlockHeader(splitBlock, pos, pos->next, foundBlockSize);

                    pos->next = splitBlock;

                    if (splitBlock->next != head)
                        splitBlock->next->prev = splitBlock;
                }

                pos->locationHi = ((uintptr_t)location) >> 14;
                pos->locationLo = (uintptr_t)location;

                return pos->data;
            }
        }

        if (pos->next == head)
            return NULL;

        pos = pos->next;
    }
}

#if RANDOLOCKE_SKIP_BAD_FREES == TRUE && !TESTING
// randolocke: a Free() the allocator can tell is wrong -- a block freed a second time, or
// a pointer whose header does not carry the magic number -- used to end in an AGB_ASSERT,
// and in the debug ROM a failed assert sends the CPU into unrelated code (see
// RANDOLOCKE_DEBUG_ASSERTS_RESUME). To a player that is a freeze.
//
// Neither case needs the heap touched to stay consistent. A block that is already free is
// already accounted for: its neighbours were merged into it the first time, so a second
// pass over it would do nothing but report. And a header without the magic number cannot
// be trusted to walk -- following its prev and next is how a bad free turns into a
// corrupted heap. So outside the test runner, which keeps the assert so a test still
// fails loudly, such a Free() is reported and skipped.
//
// The report names both ends of the bug: the code that called Free() -- look the address
// up with `arm-none-eabi-addr2line -f -e pokeemerald.elf <address>` against the ELF of
// the same build -- and, for a block freed twice, the file and line that allocated it,
// which the header still records after the first Free().
static bool32 RejectBadFree(const struct MemBlock *block, const void *caller)
{
    if (block->magic != MALLOC_SYSTEM_ID)
    {
        DebugPrintfLevel(MGBA_LOG_ERROR, "Free(0x%x) skipped: not a heap block (magic 0x%x). Called from 0x%x",
                         (u32)block->data, block->magic, (u32)caller);
        return TRUE;
    }
    if (!block->allocated)
    {
        DebugPrintfLevel(MGBA_LOG_ERROR, "Free(0x%x) skipped: already free. Called from 0x%x, allocated at %s",
                         (u32)block->data, (u32)caller,
                         (const char *)(ROM_START | (block->locationHi << 14) | block->locationLo));
        return TRUE;
    }
    return FALSE;
}
#endif

static void FreeInternal(void *heapStart, void *pointer, const void *caller)
{
    if (pointer)
    {
        struct MemBlock *head = (struct MemBlock *)heapStart;
        struct MemBlock *block = (struct MemBlock *)((u8 *)pointer - sizeof(struct MemBlock));
#if RANDOLOCKE_SKIP_BAD_FREES == TRUE && !TESTING
        if (RejectBadFree(block, caller))
            return;
#else
        AGB_ASSERT(block->magic == MALLOC_SYSTEM_ID);
        AGB_ASSERT(block->allocated == TRUE);
#endif
        block->allocated = FALSE;

        // If the freed block isn't the last one, merge with the next block
        // if it's not in use.
        if (block->next != head)
        {
            if (!block->next->allocated)
            {
                block->size += sizeof(struct MemBlock) + block->next->size;
                block->next->magic = 0;
                block->next = block->next->next;
                if (block->next != head)
                    block->next->prev = block;
            }
        }

        // If the freed block isn't the first one, merge with the previous block
        // if it's not in use.
        if (block != head)
        {
            if (!block->prev->allocated)
            {
                AGB_ASSERT(block->prev->magic == MALLOC_SYSTEM_ID);

                block->prev->next = block->next;

                if (block->next != head)
                    block->next->prev = block->prev;

                block->magic = 0;
                block->prev->size += sizeof(struct MemBlock) + block->size;
            }
        }
    }
}

static void *AllocZeroedInternal(void *heapStart, u32 size, const char *location)
{
    void *mem = AllocInternal(heapStart, size, location);

    if (mem != NULL)
    {
        if (size & 3)
            size = 4 * ((size / 4) + 1);

        CpuFill32(0, mem, size);
    }

    return mem;
}

void InitHeap(void *heapStart, u32 heapSize)
{
    sHeapStart = heapStart;
    sHeapSize = heapSize;
    PutFirstMemBlockHeader(heapStart, heapSize);
}

void PrintHeap(void)
{
    const struct MemBlock *head = HeapHead();
    const struct MemBlock *block = head;
    do
    {
        if (block->allocated)
        {
            const char *location = MemBlockLocation(block);
            if (location)
                DebugPrintf("%s: %d bytes allocated", location, block->size);
            else
                DebugPrintf("<unknown>: %d bytes allocated", block->size);
        }
        block = block->next;
    }
    while (block != head);
}

void *Alloc_(u32 size, const char *location)
{
    void *p = AllocInternal(sHeapStart, size, location);
    if (!p)
    {
        if (TESTING)
            PrintHeap();
        fatalf("%s: out of memory trying to allocate %d bytes", location, size);
    }
    return p;
}

void *AllocUnchecked_(u32 size, const char *location)
{
    return AllocInternal(sHeapStart, size, location);
}

void *AllocZeroed_(u32 size, const char *location)
{
    void *p = AllocZeroedInternal(sHeapStart, size, location);
    if (!p)
    {
        if (TESTING)
            PrintHeap();
        fatalf("%s: out of memory trying to allocate %d bytes", location, size);
    }
    return p;
}

void *AllocZeroedUnchecked_(u32 size, const char *location)
{
    return AllocZeroedInternal(sHeapStart, size, location);
}

void Free(void *pointer)
{
    FreeInternal(sHeapStart, pointer, __builtin_return_address(0));
}

const struct MemBlock *HeapHead(void)
{
    return (const struct MemBlock *)sHeapStart;
}

const char *MemBlockLocation(const struct MemBlock *block)
{
    if (!block->allocated)
        return NULL;

    return (const char *)(ROM_START | (block->locationHi << 14) | block->locationLo);
}
