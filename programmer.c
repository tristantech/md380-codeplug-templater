#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "programmer.h"
#include "utils.h"

struct manifest_row {
	int radio_id;
	char radio_name[RADIO_NAME_LEN+1];
};

int main(int argc, char **argv) {

    /* Pointer to memory-mapped codeplug file */
    struct codeplug *cp = NULL;

    /* We are given a template codeplug and manifest as input. Check them. */
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <template> <manifest> [output dir]\n", argv[0]);
        return 1;
    }

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

    int f_cp;
    if(access(argv[1], R_OK) < 0 || (f_cp = open(argv[1], O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open file %s.\n", argv[1]);
        return 1;
    }

    FILE *f_manifest;
    if(access(argv[2], R_OK) < 0 || (f_manifest = fopen(argv[2], "r")) == NULL) {
        fprintf(stderr, "Cannot open file %s.\n", argv[2]);
        return 1;
    }

    /* Copy template codeplug into memory (it's roughly a quarter megabyte) */
    cp = malloc(CODEPLUG_SIZE);
    if(cp == NULL) {
        fprintf(stderr, "Cannot allocate memory for codeplug.\n");
        return 1;
    }

    ssize_t bytes_read = read(f_cp, cp, (size_t)CODEPLUG_SIZE);
    if(bytes_read != CODEPLUG_SIZE) {
        fprintf(stderr, "Cannot copy codeplug to memory.\n");
    }

	/* Read the manifest */
	int manifest_rows = 0;
	int manifest_size = 10; //Initial array size
	struct manifest_row *manifest = (struct manifest_row *)malloc(manifest_size*sizeof(struct manifest_row));
	if(!manifest) {
		fprintf(stderr, "Cannot allocate memory for manifest.\n");
		return 1;
	}

	while(1) {
		/* Read a line from the manifest */
        char *line = NULL;
        size_t line_len = 0;
        ssize_t length = getline(&line, &line_len, f_manifest);
        if (length < 0) {
            break; /* No more lines or failure */
        }

        memset(manifest[manifest_rows].radio_name, '\0', RADIO_NAME_LEN+1);

        int res = sscanf(line, "%u %16[-_. a-zA-Z0-9]", &manifest[manifest_rows].radio_id, manifest[manifest_rows].radio_name);
        if(res == EOF || res < 2) {
            fprintf(stderr, "Error reading line %s", line);
            free(line);
            continue;
        }
        free(line);

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
	}

	fclose(f_manifest);

	/* Insert contacts -- seek to first available slot*/
	struct digcontact *contact = CP_DIGCONTACT_PTR(cp);
	for( ; CP_DIGCONTACT_DEFINED(*contact); contact++ );

	for(int i=0; i < manifest_rows; i++) {
		// Create a new Digital Contact
		contact->call_config = 0xC2;
		codeplug_id(contact->call_id, manifest[i].radio_id);
		codeplug_str(contact->call_name, 16, manifest[i].radio_name, sizeof(manifest[i].radio_name));
		contact++;
	}

	/* Begin provisioning individual codeplugs */

    uint32_t radio_id;
    char filename[4096];
    char radio_name[RADIO_NAME_LEN+1];
    int count = 0;

    for(int i=0; i < manifest_rows; i++) {

        /* Make changes in codeplug */
        codeplug_str(cp->splash_line1, 10, "Property", 8);
        codeplug_str(cp->splash_line2, 10, manifest[i].radio_name, sizeof(manifest[i].radio_name));
        codeplug_str(cp->radio_name, 16, manifest[i].radio_name, sizeof(manifest[i].radio_name));
        codeplug_id(cp->radio_id, manifest[i].radio_id);

        /* Write out new file */
        memset(filename, '\0', sizeof(filename));
        snprintf(filename, sizeof(filename), "%s/codeplug_%s.rdt", output_dir, manifest[i].radio_name);

        FILE *f_out = fopen(filename, "w");
        if(f_out == NULL) {
            fprintf(stderr, "Cannot open `%s` for writing (%s).\n", filename, strerror(errno));
            continue;
        }

        int res = fwrite(cp, 1, CODEPLUG_SIZE, f_out);
        if(res != CODEPLUG_SIZE) {
            fprintf(stderr, "Cannot write to `%s`.\n", filename);
        }
        fclose(f_out);

        count++;
    }

    printf("Done. Processed %d lines.\n", count);

    return 0;
}
