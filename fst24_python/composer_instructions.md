# Using These Prompts with Cursor Composer

## Setup Instructions

1. Create project structure:
```bash
mkdir -p fst24_python/{src,tests,docs}
```

2. Load context files in order:
   - requirements.md
   - librmn_api.md
   - design_decisions.md
   - existing_code.md

3. Present prompts sequentially:
   - 01_core_interface.md
   - 02_metadata_handler.md
   - 03_data_access.md

## Prompt Usage

1. For each implementation:
   ```
   /create implementation for [prompt name]
   ```

2. Review generated code:
   ```
   /review implementation focusing on [specific aspect]
   ```

3. Request documentation:
   ```
   /document implementation with docstrings and examples
   ```

## Tips for Best Results

1. Keep context focused on current task
2. Request specific implementations
3. Review and refine in iterations
4. Include test cases in requests

## Implementation Order

1. Core Components:
   a. Type definitions (types.py)
   b. File handling (core_file.py)
   c. Record structure (record.py)
   d. Query interface (query.py)

2. Data Access:
   a. Memory management
   b. Type conversions
   c. Grid operations

3. Integration:
   a. Polars/pandas support
   b. XArray integration
   c. Georef bridge
