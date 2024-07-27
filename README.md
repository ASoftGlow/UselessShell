# Useless Shell

## Features

- Command auto completion (Tab)
- Command syntax hints (Ctrl+Tab)
- List commands with `help`
- Users
  - Saved to file
  - Password hashing using the RSA Data Security, Inc. MD5 Message Digest Algorithm
  - Superusers
- History
  - Saved to file per user (& guest)
  - View with `history`
  - Clear with `history clear`
  - Navigation (↑ & ↓)
- Easy command declaraiton (see [cmd_defs.c](src/cmd_defs.c))

  ```c
  CMD(
    .name = "history",
    .description = "Display the session command history",
    .impl = cmd_history,
    SUB_CMDS(
      {
        .name = "clear"
      }
    )
  ),
  ```

- Input buffer editing
  - Cursor navigation (← & →)
  - Word navigation (Ctrl+← & Ctrl+→)
  - Backspace
  - Back word (Ctrl+Backspace)
  - Delete
  - Delete word (Ctrl+Del)
  - Clear (escape)
