local config = {}

config.fps = 60
config.max_log_items = 80
config.message_timeout = 6 --< @r-lyeh 3>6
config.mouse_wheel_scroll = 50 * SCALE
config.file_size_limit = 10
config.ignore_files = "^%."
config.symbol_pattern = "[%a_][%w_]*"
config.non_word_chars = " \t\n/\\()\"':,.;<>~!@#$%^&*|+=[]{}`?-"
config.undo_merge_timeout = 0.3
config.max_undos = 10000
config.highlight_current_line = true
config.line_height = 1.2
config.indent_size = 2
config.tab_type = "soft"
config.line_limit = 80
config.project_scan_rate = 5 -- seconds to wait before running next full tree globber
config.project_scan_depth = 8 -- max folder depth tree
config.project_max_files_per_folder = 2000
config.blink_period = 1.3 --< https://github.com/rxi/lite/issues/235
config.tabs_allowed = true --< https://github.com/rxi/lite/issues/191

return config
