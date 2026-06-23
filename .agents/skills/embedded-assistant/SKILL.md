---
name: embedded-assistant
description: Assist with generating, refactoring, or debugging embedded C/C++ code, especially MCU drivers, peripheral abstractions, hardware wrappers, chip drivers, firmware modules, and upstream business-code interfaces. Use when the user asks in Chinese or English to develop, build, write, create, refactor, review, debug, diagnose, locate a bug, or fix behavior in a peripheral driver, chip driver, hardware abstraction, firmware module, embedded module, or C/C++ module, including prompts like "开发一个xxx外设驱动", "开发xxx芯片驱动", "生成xxx驱动", "重构xxx模块", "调试xxx模块", "定位下面嵌入式代码的bug", "develop a xxx peripheral driver", "develop a xxx chip driver", "generate xxx driver", "refactor xxx module", "debug xxx module", or "find the bug in this embedded code".
---

# Embedded Assistant

## Skill-local Resources

This skill may use resource files placed in the same directory as this `SKILL.md`. Before searching the target project for project-specific rules, first look only in this skill's own directory for relevant local resource files, such as coding-standard, refactoring-rule, debug-scenario, platform, MCU, board, vendor, or helper-script files. Do not read resource files from other skill directories.

Treat skill-local resource files as default guidance for this skill. If target-project rules are also found and conflict with skill-local resources, surface the conflict and ask the user which rule should win.

## Core Workflow

Use an approval-gated workflow before changing code. Send each workflow phase as a separate message and stop after each phase until the user explicitly confirms. Do not combine intent analysis, dependency confirmation, interface definition, interface safety review, and refactoring into one response.

1. Determine whether the user wants to refactor existing code, generate embedded code from scratch, or enter the debug workflow.
2. Identify the exact module the user asked to generate or refactor. If the target module is ambiguous, ask for the module name before continuing.
3. For refactoring, if the user did not provide code, ask for the relevant source, headers, build errors, and target constraints. Do not invent missing code.
4. For from-scratch generation, ask for the intended behavior, target MCU/toolchain, language standard, platform constraints, and integration style. If generating a driver for a specific chip, require the chip manual/datasheet before designing the driver.
5. When a chip manual is required, ask the user to place it in the project if it is not already available. Tell the user the manual should have the same base name as the chip model, such as `ADS1110.pdf` for `ADS1110`, so it can be found automatically. Search the project for the same-named manual before asking for details manually.
6. Before analyzing interfaces, proposing designs, reviewing code, or editing files, first read relevant resource files from this skill's own directory only. Then check whether the user or target project provides an embedded code specification, coding standard, refactoring rule file, or similar project convention document. Search in the current workspace/project context rather than relying on a fixed path or fixed filename. Do not read resources from other skill directories. If a relevant project specification exists, read it and treat it as project-specific guidance. If the user refers to a specification but it cannot be found, ask the user where it is.
7. Read the existing code or generation requirements and state the macroscopic intent: what the file/module will do, each class's responsibility, and what each public interface contributes to that responsibility. Avoid reducing intent analysis to event traces such as "A calls B, then B calls C" unless call order is itself part of the domain behavior. Stop and wait for confirmation.
8. If the task depends on other project files, first try to find and read those dependencies in the project. Prefer existing abstract peripheral, hardware, and protocol interfaces over concrete low-level entities. For example, before using a vendor serial handle directly, search for an abstract serial interface; before modeling a reset pin as a raw GPIO or vendor-specific pin type, search for an abstract pin interface. If a dependency cannot be found, ask the user for the missing classes, functions, thread-safety guarantees, ownership rules, side effects, and semantics before relying on assumptions. Present dependency findings separately and stop for confirmation.
9. Do not modify or generate code until the intent and dependency semantics are clear enough to preserve behavior or desired behavior. If the user corrects the interpretation, update the understanding and stop again for confirmation.
10. Before formal refactoring or generation, draft the proposed interface definitions: names, types, ownership, error model, blocking behavior, initialization flow, comments, and migration notes. Apply any project specification that was found or provided. Wait for the user to approve the interface before continuing.
11. After interface approval, ask the user to confirm the thread-safety expectations of each interface, including use from main loop, ISR, DMA callbacks, RTOS tasks, and shared state access. Treat single-core embedded systems as concurrent when interrupts or callbacks exist. Stop and wait for confirmation.
12. Before attempting any file edit, ask the user for explicit permission to create or modify the exact target files. Do not modify files while the user is unaware or before permission is granted.
13. Refactor or generate according to the priority order in this skill and any project specification that was found or provided. Do not begin implementation until the prior phase confirmations and file-edit permission are complete.
14. After changes, summarize what changed by priority level and call out any assumptions, unverified hardware behavior, thread-safety decisions, dependency gaps, or tests/builds that could not be run.
15. After generation or refactoring, provide the minimal example code needed to exercise the generated/refactored module. Do not modify business code or integration code for the user; tell the user to copy the example manually if they want to try it. Ask whether the example or module has any bug. If the user reports a bug, enter the debug workflow.

## File Modification Scope

Constrain file edits to the user-requested module.

- If the user asks to generate module `xxx`, create only files for that module: `xxx.h`, `xxx.hpp`, `xxx.inl`, `xxx.c`, and/or `xxx.cpp`.
- If the user asks to refactor module `xxx`, modify only that module's files: `xxx.h`, `xxx.hpp`, `xxx.inl`, `xxx.c`, and/or `xxx.cpp`.
- Do not edit other modules, build files, shared utilities, dependency files, generated files, tests, docs, or configuration unless the user explicitly expands the requested scope.
- Reading dependency files is allowed for understanding. Writing them is forbidden unless the user explicitly asks to include them in the edit scope.
- Before modifying or creating any file, state the exact paths that would be changed and ask for permission. Proceed only after the user grants permission.
- If a necessary fix appears to require changes outside the permitted module, stop and explain the dependency or integration issue instead of editing outside scope.

## Project Specification

Project-specific specifications may live under different names or locations. Do not hardcode a path, filename, or constraint document name.

- If the user mentions a specification, coding standard, rule file, constraint file, or team convention, locate and read that file before using the rules.
- If the project contains an obvious specification or coding-standard file, read it before code analysis, interface design, code review, generation, refactoring, or final safety review.
- Treat the found or user-provided specification as project-specific guidance. If it conflicts with this skill, surface the conflict and ask the user which rule should win.
- If no specification is found and the user did not mention one, continue with this skill's built-in rules.

## Project Scenario YAML

Project and platform specific debug knowledge may live in this skill's own local resources or in the target project.

- Before detailed debugging, first search only this skill's own directory for relevant scenario, debug, diagnosis, platform, MCU, board, or vendor YAML files. Do not read YAML files from other skill directories.
- Then search the current target project for scenario, debug, diagnosis, platform, MCU, board, or vendor YAML files.
- If a relevant scenario YAML exists, read it and apply it as context for likely failure modes and verification order.
- Do not embed project-specific or vendor-specific troubleshooting lists directly in this `SKILL.md`; keep them in skill-local or target-project scenario YAML files.
- If the user mentions a project/platform scenario file but it cannot be found, ask where it is.

## Debug Workflow

Use this workflow when the user asks to debug first, or when the user reports a bug after generation/refactoring and the minimal example.

1. Read the bug description, observed behavior, logs, target chip or board, peripheral or module, relevant code, configuration clues, and minimal reproduction.
2. Search for and read relevant project specifications and scenario YAML files. Keep the debug flow generic in this skill; use the YAML files for project-specific issue lists.
3. Decide whether the case is simple or complex.
4. For short descriptions or typical known issues, provide the most likely cause, a direct solution direction, and a verification method.
5. For complex cases, provide only the most likely cause, evidence, and the next verification step first. Do not provide a full repair procedure until the user asks to continue or confirms the diagnosis direction.
6. If project files need inspection, read only. Do not edit files.
7. If code or configuration changes are needed, state the exact files or settings, why the change is needed, and the risk. Wait for user approval before editing.
8. If an edit is approved, keep the same module-scope and file-permission rules as generation/refactoring.

For short or typical issues, answer with:

```text
Most likely cause:
...

Solution:
...

Verification:
...
```

For complex issues, answer with:

```text
Most likely cause:
...

Evidence:
...

Next verification:
...
```
