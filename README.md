# Building
Simply invoke the provided Makefile to build the templater

# Usage
Basic usage:

```
./templater <template> <manifest>
```

 * `template`: Path to an MD380 codeplug to read as the template file.
 * `manifest`: Path to a text file containing `<Radio ID> <Radio Name>`
    records, one per line. `<Radio ID>` must be a 24-bit unsigned integer.
    `<Radio Name>` is a maximum 16-char string.

The generated codeplugs are placed into the current working directory.