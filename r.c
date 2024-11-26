#include "vmlib.h"
/* find the best fitting free block */
static struct block_header *find_best_fit(size_t required_size) {
    struct block_header *current = heapstart;
    struct block_header *best_fit = NULL;
    size_t smallest_fit = SIZE_MAX;  // Start with maximum possible size
    // if starts from small size, the first block that meets the requirement
    // would automatically be considered the best, even if later blocks
    // might be a better fit

    // Traverse the heap until we hit the end mark
    while (current->size_status != VM_ENDMARK) {
        size_t current_size = BLKSZ(current); //BLKSZ masks out the last
                                              // two bits to get pure size
        int is_free = !(current->size_status & VM_BUSY);
        // if the block is busy -> return non-zero and false
        // if the block is free -> return zero and true

        // Check if this block is free and big enough
        if (is_free && current_size >= required_size) {
            // If this is the best (smallest) fit so far, remember it
            if (current_size < smallest_fit) {
                best_fit = current;
                smallest_fit = current_size;
            }
        }

        // Move to next block
        current = (struct block_header *)((char *)current + current_size);
    }

    return best_fit;
}

/* Helper function to split a block if necessary */
static void split_block(struct block_header *block, size_t required_size) {
    size_t total_size = BLKSZ(block);
    size_t remaining_size = total_size - required_size;

    // Only split if the remaining size is large enough for a new block
    // (need space for header + minimum payload)
    if (remaining_size >= BLKALIGN + sizeof(struct block_header)) {
        // Modify the current block to be the allocated block
        // Set size of allocated block
        block->size_status = required_size;
        block->size_status |= (VM_BUSY | (block->size_status | VM_PREVBUSY));
        // sets the LSB as busy, and preserves the previous block's busy stat

        // Create new free block from remaining space
        struct block_header *new_block =
            (struct block_header *)((char *)block + required_size);

        // Set the size of the new free block
        new_block->size_status = remaining_size;
        new_block->size_status |= VM_PREVBUSY;  // Previous block (the allocated one in this function) is busy

        // Add footer to the new free block
        struct block_footer *new_footer =
            (struct block_footer *)((char *)new_block + remaining_size - sizeof(struct block_footer));

        new_footer->size = remaining_size;
    } else {
        // Not enough space to split - allocate entire block
        block->size_status |= VM_BUSY;
    }
}

void *vmalloc(size_t size) {
    // Check for invalid size
    if (size == 0) {
        return NULL;
    }

    // Calculate required block size (including header)
    // and round up to multiple of 16
    // ROUND_UP(N, S) ((((N) + (S)-1) / (S)) * (S))
    // BLKALIGN = alignment boundary for blocks = 16
    size_t required_size = ROUND_UP(size + sizeof(struct block_header), BLKALIGN); // rounded up to the nearest multiple of 16 and include space for
       // block header

    // Find the best fitting block
    struct block_header *best_block = find_best_fit(required_size);
    if (!best_block) {
        return NULL;  // No suitable block found
    }

    // Split the block if necessary and mark as allocated
    split_block(best_block, required_size);

    // Update the "previous block busy" bit of the next block
    struct block_header *next_block =
        (struct block_header *)((char *)best_block + BLKSZ(best_block));
    if (next_block->size_status != VM_ENDMARK) {
        next_block->size_status |= VM_PREVBUSY;
    }

    // Return pointer to payload area (after header)
    return (void *)((char *)best_block + sizeof(struct block_header));
}
