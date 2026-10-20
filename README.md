# 🏠 HomeLogic — A Smart Home Automation Language

> **A domain-specific programming language for simulating smart home automation, built with LEX & YACC as a Compiler Design project (Semester 7).**

HomeLogic lets you **declare devices, sensors, and variables**, write **conditional logic and routines**, **schedule actions** on a virtual clock, and **simulate an entire day** of a smart home — all through a clean, English-like syntax compiled and executed by a custom interpreter.

---

## 📋 Table of Contents

- [Features](#-features)
- [Language Quick Reference](#-language-quick-reference)
- [Project Architecture](#-project-architecture)
- [Compiler Phases](#-compiler-phases)
- [Build Instructions](#-build-instructions)
- [Running Programs](#-running-programs)
- [Example: Valid Program Output](#-example-valid-program-output)
- [Example: Error Detection Output](#-example-error-detection-output)
- [File Structure](#-file-structure)
- [How It Works (End to End)](#-how-it-works-end-to-end)
- [License](#-license)

---

## ✨ Features

| Category | Capability |
|---|---|
| **Declarations** | `DEVICE`, `SENSOR`, `VAR` — typed symbol registration with duplicate detection |
| **Device Control** | `SWITCHON`, `SWITCHOFF`, `DIM` (0–100%) with idempotency warnings |
| **Sensor Input** | `SENSE` reads sensor values at runtime with 3-attempt validation |
| **User Input** | `INPUT` reads variable values interactively |
| **Output** | `SHOW` prints strings or evaluated expressions |
| **Conditionals** | `WHEN ... OTHERWISE` (nested supported) |
| **Loops** | `REPEAT n { ... }` with 1000-iteration safety cap |
| **Routines** | Named `ROUTINE` blocks, invoked with `RUN` |
| **Scheduling** | `AT HH:MM { ... }` schedules actions; `CLOCK HH:MM` advances virtual time |
| **Expressions** | Arithmetic (`+ - * /`), comparison (`== != < > <= >=`), logical (`AND OR NOT`) |
| **Error Handling** | Lexical, syntax, and semantic errors — all with line numbers |
| **Symbol Table** | Tracks state, value, level, and kind for every declared name |

---

## 📖 Language Quick Reference

### Declarations
```
DEVICE hall_light;          // declare a controllable device
SENSOR temperature;         // declare a sensor (readable)
SENSOR humidity = 50;       // declare + initialize
VAR fan_speed = 0;          // declare a general variable
```

### Device Control
```
SWITCHON hall_light;        // turn ON
SWITCHOFF hall_light;       // turn OFF
DIM hall_light 75;          // set brightness/speed to 75%
```

### Input / Output
```
SENSE temperature;          // prompt user for sensor reading
INPUT fan_speed;            // prompt user for variable value
SHOW "Hello, world!";      // print a string
SHOW temperature;           // print a value
SHOW temperature + 10;      // print an expression result
```

### Control Flow
```
WHEN (temperature > 30) {
    SWITCHON ac_unit;
}
OTHERWISE {
    SWITCHOFF ac_unit;
}

REPEAT 3 {
    SWITCHON porch_light;
    SWITCHOFF porch_light;
}
```

### Routines
```
ROUTINE morning_setup {
    SWITCHON hall_light;
    SWITCHON kitchen_light;
}

RUN morning_setup;          // call the routine
```

### Scheduling & Clock
```
AT 06:30 { RUN morning_setup; }     // register a timed action
AT 22:00 { RUN night_mode; }

CLOCK 06:30;                         // advance virtual clock → fires matching AT rules
CLOCK 22:00;
```

### Comments
```
// This is a single-line comment
```

---

## 🏗 Project Architecture

```
                  ┌─────────────┐
   source.home ──▸│  LEX (Flex)  │──▸ Token Stream
                  └──────┬──────┘
                         │
                  ┌──────▼──────┐
                  │ YACC (Bison)│──▸ AST (Abstract Syntax Tree)
                  └──────┬──────┘
                         │
              ┌──────────▼──────────┐
              │  Semantic Analyzer  │──▸ Symbol Table + Error Reports
              └──────────┬──────────┘
                         │
              ┌──────────▼──────────┐
              │  Tree-Walk Executor │──▸ Simulated Smart Home Output
              └─────────────────────┘
```

---

## 🔬 Compiler Phases

### Phase 1 — Lexical Analysis (`HomeLogic.l`)
The Flex-based lexer tokenizes `.home` source files:
- **Keywords**: `DEVICE`, `SENSOR`, `VAR`, `SWITCHON`, `SWITCHOFF`, `DIM`, `SENSE`, `INPUT`, `SHOW`, `WHEN`, `OTHERWISE`, `REPEAT`, `ROUTINE`, `RUN`, `AT`, `CLOCK`, `AND`, `OR`, `NOT`
- **Literals**: integers, time values (`HH:MM`), quoted strings
- **Identifiers**: `[a-zA-Z_][a-zA-Z0-9_]*`
- **Operators**: `==`, `!=`, `<=`, `>=`, `<`, `>`, `=`, `+`, `-`, `*`, `/`
- **Comments**: `// ...` are stripped
- **Error reporting**: Stray/invalid characters flagged with line number

### Phase 2 — Syntax Analysis (`HomeLogic.y`)
The Bison-based parser validates structure against the grammar and builds an AST:
- **Declarations** → register names in the symbol table
- **Actions** → device control, I/O, routine calls, clock operations
- **Control flow** → `WHEN/OTHERWISE`, `REPEAT`, `AT` schedules
- **Expressions** → full precedence with `AND/OR/NOT`, arithmetic, comparisons
- **Error recovery** → `error ';'` rule allows parsing to continue past bad statements

### Phase 3 — Semantic Analysis (embedded in `HomeLogic.y`)
Checks performed during parsing:
- Duplicate declaration detection
- Undeclared name usage
- Kind mismatch (e.g., `SWITCHON` on a `SENSOR`)
- Invalid time values (must be `00:00`–`23:59`)
- Assigning to devices with `=` (must use `SWITCHON`/`DIM`)
- Using routines in expressions

### Phase 4 — Execution (Tree-Walk Interpreter)
If zero errors, the AST is executed:
- **Virtual clock** tracks simulated time
- **AT rules** fire when `CLOCK` advances to their scheduled time
- **Runtime checks**: division by zero, DIM range (0–100), input validation (3 attempts)
- **Symbol table dump** prints the final state of all declared names

---

## 🔧 Build Instructions

### Prerequisites
| Tool | Purpose | Install |
|---|---|---|
| **Flex** (≥ 2.6) | Lexical analyzer generator | `apt install flex` / `brew install flex` / [Win flex-bison](https://github.com/lexxmark/winflexbison) |
| **Bison** (≥ 3.0) | Parser generator | `apt install bison` / `brew install bison` / [Win flex-bison](https://github.com/lexxmark/winflexbison) |
| **GCC** | C compiler | `apt install gcc` / `brew install gcc` / [MinGW](https://www.mingw-w64.org/) |

### Build Commands

```bash
# Step 1: Generate parser (creates HomeLogic.tab.c and HomeLogic.tab.h)
bison -d HomeLogic.y

# Step 2: Generate lexer (creates lex.yy.c)
flex HomeLogic.l

# Step 3: Compile everything into the executable
gcc HomeLogic.tab.c lex.yy.c -o homelogic
```

After this, you'll have the `homelogic` executable (or `homelogic.exe` on Windows).

---

## ▶️ Running Programs

```bash
# Run a valid smart home program (interactive — prompts for sensor/variable input)
./homelogic program.home

# Run the error test file (demonstrates all error types)
./homelogic error.home
```

---

## ✅ Example: Valid Program Output

Running `./homelogic program.home` with inputs `35` (temperature), `60` (humidity), and `50` (fan speed):

```
========================================
        HomeLogic  v1.0  (.home)
========================================

>> Checking program
[Added to home] DEVICE 'hall_light'
[Added to home] DEVICE 'hall_fan'
[Added to home] DEVICE 'bedroom_light'
[Added to home] DEVICE 'bedroom_fan'
[Added to home] DEVICE 'kitchen_light'
[Added to home] DEVICE 'porch_light'
[Added to home] DEVICE 'bathroom_light'
[Added to home] DEVICE 'ac_unit'
[Added to home] SENSOR 'temperature'
[Added to home] SENSOR 'humidity'
[Added to home] VARIABLE 'temp_limit'
[Added to home] VARIABLE 'humid_limit'
[Added to home] VARIABLE 'fan_speed'
[Added to home] VARIABLE 'hour'
[Added to home] VARIABLE 'alert_count'
[Added to home] ROUTINE 'morning_setup'
[Added to home] ROUTINE 'night_mode'
[Added to home] ROUTINE 'all_off'

>> Starting the virtual home
[Scheduled] rule for 06:30 stored.
[Scheduled] rule for 22:00 stored.
[Scheduled] rule for 23:59 stored.
[Scheduled] rule for 07:00 stored.
[Scheduled] rule for 07:30 stored.
[Scheduled] rule for 18:00 stored.
[00:00] ========================================
[00:00]    Welcome to Smart Home Simulator
[00:00] ========================================
[00:00]
[00:00] Please provide current sensor readings:
Enter sensor value for temperature:   [OK] temperature = 35
Enter sensor value for humidity:   [OK] humidity = 60
[00:00]
[00:00] Current readings:
[00:00] 35
[00:00] 60
[00:00] It is hot! Starting cooling system.
[00:00] bedroom_fan -> ON
[00:00] bedroom_fan level set to 75%
[00:00] Fan should be enough for now.
[00:00] Humidity is normal.
[00:00]
[00:00] Set your preferred fan speed (0-100):
Enter variable value for fan_speed:   [OK] fan_speed = 50
[00:00] hall_fan level set to 50%
[00:00] Fan speed set.
[00:00] 50
[00:00]
[00:00] Running security light check (3 blinks):
[00:00] porch_light -> ON
[00:00] porch_light -> OFF
[00:00] porch_light -> ON
[00:00] porch_light -> OFF
[00:00] porch_light -> ON
[00:00] porch_light -> OFF
[00:00] Security checks done:
[00:00] 3
[00:00]
[00:00] Setting up movie mode...
[00:00] hall_light level set to 20%
[00:00] bedroom_light level set to 0%
[00:00] kitchen_light is already OFF.
[00:00] Enjoy the movie!
[00:00]
[00:00] === Simulating a full day ===
[06:30] Clock moved to 06:30
[06:30] schedule fired
[06:30] Running routine morning_setup
[06:30] --- Morning Routine ---
[06:30] hall_light is already ON.
[06:30] kitchen_light -> ON
[06:30] hall_fan is already ON.
[06:30] bedroom_light level set to 40%
[06:30] porch_light is already OFF.
[06:30] Good morning! Lights are on, fan started.
[07:00] Clock moved to 07:00
[07:00] schedule fired
[07:00] Bathroom light on for morning routine.
[07:00] bathroom_light -> ON
[07:30] Clock moved to 07:30
[07:30] schedule fired
[07:30] bathroom_light -> OFF
[07:30] Bathroom light off. Heading out!
[18:00] Clock moved to 18:00
[18:00] schedule fired
[18:00] Evening: Switching on hall and kitchen lights.
[18:00] hall_light is already ON.
[18:00] kitchen_light is already ON.
[18:00] hall_light level set to 80%
[22:00] Clock moved to 22:00
[22:00] schedule fired
[22:00] Running routine night_mode
[22:00] --- Night Routine ---
[22:00] hall_light -> OFF
[22:00] kitchen_light -> OFF
[22:00] hall_fan -> OFF
[22:00] bedroom_light level set to 10%
[22:00] porch_light -> ON
[22:00] ac_unit is already OFF.
[22:00] Good night! Everything dimmed/off, porch light on.
[23:59] Clock moved to 23:59
[23:59] schedule fired
[23:59] Running routine all_off
[23:59] --- Emergency: All Off ---
[23:59] hall_light is already OFF.
[23:59] hall_fan is already OFF.
[23:59] bedroom_light -> OFF
[23:59] bedroom_fan -> OFF
[23:59] kitchen_light is already OFF.
[23:59] porch_light -> OFF
[23:59] bathroom_light is already OFF.
[23:59] ac_unit is already OFF.
[23:59] All devices turned off.
[23:59]
[23:59] === Day simulation complete ===

---- Final state of every name ----
NAME             TYPE       NOW
hall_light       DEVICE     OFF (level 80%)
hall_fan         DEVICE     OFF (level 50%)
bedroom_light    DEVICE     OFF (level 10%)
bedroom_fan      DEVICE     OFF (level 75%)
kitchen_light    DEVICE     OFF (level 100%)
porch_light      DEVICE     OFF (level 100%)
bathroom_light   DEVICE     OFF (level 100%)
ac_unit          DEVICE     OFF (level 0%)
temperature      SENSOR     35
humidity         SENSOR     60
temp_limit       VARIABLE   32
humid_limit      VARIABLE   70
fan_speed        VARIABLE   50
hour             VARIABLE   0
alert_count      VARIABLE   3
morning_setup    ROUTINE    defined
night_mode       ROUTINE    defined
all_off          ROUTINE    defined

Simulation complete.
```

---

## ❌ Example: Error Detection Output

Running `./homelogic error.home` demonstrates all three error types:

```
========================================
        HomeLogic  v1.0  (.home)
========================================

>> Checking program
[Added to home] DEVICE 'fan'
[Added to home] SENSOR 'temp'
HomeLogic semantic error, line 3: 'fan' is already declared.
HomeLogic semantic error, line 4: 'temp' is a SENSOR, but a DEVICE is required here.
HomeLogic semantic error, line 5: 'lamp' is not declared.
HomeLogic semantic error, line 6: 'fan' is a DEVICE, but a SENSOR is required here.
HomeLogic semantic error, line 7: 'night' is not declared.
HomeLogic semantic error, line 8: Invalid time 25:70 (use 00:00 to 23:59).
HomeLogic semantic error, line 9: 'fan' is a DEVICE/ROUTINE; use SWITCHON, SWITCHOFF or DIM instead of '='.
HomeLogic syntax error, line 10 -> syntax error, unexpected ';'
HomeLogic lexical error, line 12: stray character '@'

Build stopped: 8 error(s) found, nothing was executed.
```

**Error types caught:**

| Line | Error Type | Description |
|------|-----------|-------------|
| 3 | Semantic | Duplicate declaration (`DEVICE fan` declared twice) |
| 4 | Semantic | Kind mismatch (`SWITCHON` used on a `SENSOR`) |
| 5 | Semantic | Undeclared identifier (`lamp` never declared) |
| 6 | Semantic | Kind mismatch (`SENSE` used on a `DEVICE`) |
| 7 | Semantic | Undeclared routine (`night` never declared) |
| 8 | Semantic | Invalid time value (`25:70` is out of range) |
| 9 | Semantic | Assignment to device (must use `SWITCHON`/`DIM`) |
| 10 | Syntax | Unexpected token (`SHOW 10 / ;` is incomplete) |
| 12 | Lexical | Stray character (`@` is not valid in HomeLogic) |

---

## 📁 File Structure

```
HomeLogic/
├── HomeLogic.l          # Lexer specification (Flex)
├── HomeLogic.y          # Parser + semantic analysis + interpreter (Bison)
├── program.home         # Full demo: 2BHK smart home, full-day simulation
├── error.home           # Test file with intentional errors (all 3 types)
├── .gitignore           # Ignores generated build artifacts
└── README.md            # This documentation
```

**Generated at build time (not committed):**
- `lex.yy.c` — Generated C code from Flex
- `HomeLogic.tab.c` — Generated C code from Bison
- `HomeLogic.tab.h` — Token definitions header
- `homelogic.exe` — Compiled executable

---

## 🔄 How It Works (End to End)

```
1. You write a .home file using HomeLogic syntax
                  │
2. LEX (Flex) tokenizes it into keywords, numbers, strings, operators
                  │
3. YACC (Bison) parses tokens against the grammar
   ├── Builds an AST (Abstract Syntax Tree)
   ├── Registers declarations in the Symbol Table
   └── Runs semantic checks (type mismatches, undeclared names, etc.)
                  │
4. If 0 errors → Tree-Walk Interpreter executes the AST
   ├── Controls virtual devices (ON/OFF/DIM)
   ├── Reads sensor/variable inputs from user
   ├── Evaluates expressions and conditionals
   ├── Runs routines and loops
   ├── Fires scheduled AT rules when CLOCK advances
   └── Prints final Symbol Table state
                  │
5. If errors → Halts with detailed error report (line numbers + messages)
```

---

## 📄 License

This project was created as a **Compiler Design (CD) mini-project** for academic purposes.

---

<p align="center"><b>Built with ❤️ using LEX, YACC, and C</b></p>
