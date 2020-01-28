#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "programmer.h"
#include "utils.h"

struct manifest_row {
    uint32_t radio_id;
    char radio_name[RADIO_NAME_LEN];
};

int main(int argc, char **argv) {
    // We are given a template codeplug and manifest as input. Check them.
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <template> <manifest> [output dir]\n", argv[0]);
        return 1;
    }

    // Verify the template ends in .rdt
    if(!strstr(argv[1], ".rdt")) {
        fprintf(stderr, "Error: Codeplug file does not end in .rdt\nDid you swap the arguments?\n");
        return 1;
    }

    char *output_dir;
    if(argc >= 4) {
        // User specified an output directory.
        output_dir = argv[3];
    } else {
        // Use working dir if unspecified.
        output_dir = ".";
    }

    FILE *f_cp;
    if(access(argv[1], R_OK) < 0 || (f_cp = fopen(argv[1], "rb")) < 0) {
        fprintf(stderr, "Cannot open file %s.\n", argv[1]);
        return 1;
    }

    FILE *f_manifest;
    if(access(argv[2], R_OK) < 0 || (f_manifest = fopen(argv[2], "r")) == NULL) {
        fprintf(stderr, "Cannot open file %s.\n", argv[2]);
        return 1;
    }

    // Copy template codeplug into memory (it's roughly a quarter megabyte)
    struct codeplug *cp = malloc(CODEPLUG_SIZE);
    if(cp == NULL) {
        fprintf(stderr, "Cannot allocate memory for codeplug.\n");
        return 1;
    }
    ssize_t bytes_read = fread(cp, sizeof(uint8_t), (size_t)CODEPLUG_SIZE, f_cp);
    if(bytes_read != CODEPLUG_SIZE) {
        fprintf(stderr, "Cannot copy codeplug to memory.\n");
    }

    // Read the manifest
    size_t manifest_rows = 0;
    size_t manifest_size = 10; //Initial array size
    struct manifest_row *manifest = malloc(manifest_size*sizeof(struct manifest_row));
    if(!manifest) {
        fprintf(stderr, "Cannot allocate memory for manifest.\n");
        return 1;
    }

    int line_counter = 1;
    while(1) {
        // Read a line from the manifest
        char *line = NULL;
        size_t line_len = 0;
        ssize_t length = getline(&line, &line_len, f_manifest);
        if (length < 0) {
            break; // No more lines, or failure
        }

        // Strip newlines
        for (size_t i = 0; i < strlen(line); i++) {
            if (line[i] == '\n' || line[i] == '\r') {
                line[i] = '\0';
                break;
            }
        }

        if (*line == '#' || strlen(line) < 3) {
            // Comment or line too short. Ignore.
            goto cleanup_row;
        }

        // Break the line into two parts at the first space (' ').
        // Preserve the value of `line` so we can properly free() it later.
        char *radio_name_str = line;
        char *radio_id_str = strsep(&radio_name_str, " ");

        if (!radio_name_str) {
            // Couldn't find space seperator.
            fprintf(stderr, "Error reading line %d\n", line_counter);
            goto cleanup_row;
        }

        // Parse the radio ID string into an unsigned integer (base 10)
        char *endptr = NULL;
        errno = 0;
        manifest[manifest_rows].radio_id = strtoul(radio_id_str, &endptr, 10);
        if (radio_id_str == endptr || errno != 0) {
            // Conversion error
            fprintf(stderr, "Error parsing Radio ID on line %d: %s.\n", line_counter, strerror(errno));
            goto cleanup_row;
        }

        if (manifest[manifest_rows].radio_id >= (1<<24)) {
            // Out of bounds for 24-bit int
            fprintf(stderr, "Invalid 24-bit integer on line %d.\n", line_counter);
            goto cleanup_row;
        }

        // Copy the radio name
        strncpy(manifest[manifest_rows].radio_name, radio_name_str, RADIO_NAME_LEN);

        manifest_rows++;

        if(manifest_rows == manifest_size) {
            // Expand the array
            manifest_size += 10;
            manifest = realloc(manifest, manifest_size*(sizeof(struct manifest_row)));
            if(!manifest) {
                fprintf(stderr, "Cannot allocate memory for manifest.\n");
                return 1;
            }
        }

cleanup_row:
        line_counter++;
        free(line);
    }

    fclose(f_manifest);

    // Insert contacts -- seek to first available slot
    struct digcontact *contact = CP_DIGCONTACT_PTR(cp);
    for( ; CP_DIGCONTACT_DEFINED(*contact); contact++ );

    for(int i=0; i < manifest_rows; i++) {
        // Create a new Digital Contact
        contact->call_config = 0xC2; // Private call, call recv tone
        codeplug_id(contact->call_id, manifest[i].radio_id);
        codeplug_str(contact->call_name, 16, manifest[i].radio_name, sizeof(manifest[i].radio_name));
        contact++;
    }

    //
    // Begin provisioning individual codeplugs
    //

    char filename[PATH_MAX];
    int count = 0;

    for(int i=0; i < manifest_rows; i++) {
        // Make changes in codeplug. Splash line 1 is left alone.
        codeplug_str(cp->splash_line2, 10, manifest[i].radio_name, sizeof(manifest[i].radio_name));
        codeplug_str(cp->radio_name, 16, manifest[i].radio_name, sizeof(manifest[i].radio_name));
        codeplug_id(cp->radio_id, manifest[i].radio_id);

        // Write out new file
        memset(filename, '\0', sizeof(filename));
        snprintf(filename, sizeof(filename), "%s/codeplug_%u_%.16s.rdt", output_dir,
                 manifest[i].radio_id, manifest[i].radio_name);

        FILE *f_out = fopen(filename, "wb");
        if(f_out == NULL) {
            fprintf(stderr, "Cannot open `%s` for writing (%s).\n", filename, strerror(errno));
            continue;
        }

        size_t res = fwrite(cp, sizeof(uint8_t), CODEPLUG_SIZE, f_out);
        if(res != CODEPLUG_SIZE) {
            fprintf(stderr, "Cannot write to `%s`.\n", filename);
        }
        fclose(f_out);

        count++;
    }

    printf("Done. Processed %d lines.\n", count);

    return 0;
}
