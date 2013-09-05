macro-ram-machine
=================

Computation model emulator (infinite memory, infinite variable size, commands inc n, dec n w, goto w, stop, with macros)

[0|1|2|3|...]

- each register contains a natural number (limited by the type of 'adress', which is some sort of int)
- macros supported by push_macro("name", arg_count, "commands separated by whitespace")

inc n: [n] = [n] + 1;
goto k: goto k;
dec n k: if ([n] != 0) [n] = [n] - 1; else goto k;
stop: return;

---

Use QT Creator IDE to build. Requires QT libs.
