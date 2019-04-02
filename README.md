# Building
Simply invoke the provided Makefile to build the templater

# Usage
Basic usage:

```
./templater <template> <manifest> [output dir]
```

 * `template`: Path to an MD380 codeplug to read as the template file.
 * `manifest`: Path to a text file containing `<Radio ID> <Radio Name>`
    records, one per line. `<Radio ID>` must be a 24-bit unsigned integer.
    `<Radio Name>` is a maximum 16-char string. These are used to create
    individual codeplugs and also build a contact list.
 * `output dir` (optional) Specifies an output directory for the generated
    codeplugs. If omitted, the current working directory is used.
