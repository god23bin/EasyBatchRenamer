## Description

A easy tool that allow you to rename assets in Content Browser and actors in Outliner.

Supported languages: Chinese, English, Japanese.

Add to your library on Fab for free: https://www.fab.com/listings/9bdaf3ce-7f75-4bbf-a4ea-21cea09ee5f7

在虚幻商城 Fab 中将该插件免费添加到你的库中：https://www.fab.com/listings/9bdaf3ce-7f75-4bbf-a4ea-21cea09ee5f7

---

### Usage Instructions:

1.Enable the EasyBatchRenamer plugin and restart the Unreal Editor.

2.Open the Tools menu, locate EasyBatchRenamer under the Tool category.

3.Select the items to be batch-renamed, such as assets in the Content Browser, Actors in the Outliner, or objects directly in the Viewport.

4.Enter renaming rules, such as search text, replacement text, whether to add prefixes/suffixes, etc.

5.Click the "Apply Rename" button and wait for the process to complete.

---

#### 使用方式：

1.开启 EasyBatchRenamer 插件并重启虚幻编辑器

2.打开工具菜单，找到 Tool 分类中的 EasyBatchRenamer

3.选中需要批量重命名的条目，比如内容浏览器中的资产，大纲视图中的Actor，也可以直接点击视口中的对象

4.输入重命名规则，比如你要查找的文本，替换的文本，是否添加前缀、后缀等等

5.点击应用重命名按钮，等待完成重命名

---

### Features:

- Support real-time preview of renamed names.

- Support manual naming with priority over renaming rules.

- Support renaming both assets and Actors simultaneously.

- Support ordinary text search with case sensitivity.

- Support regular expression search and capture group references.

- Support adding prefixes and suffixes.

- Support numbering.

- Support case conversion (valid for English letters only): No Change, Replaced Part To Lower, Replaced Part To Upper, Whole Name To Lower, Whole Name To Upper.

---

#### 功能：

- 支持实时预览重命名后的名称

- 支持手动命名，优先级高于重命名规则

- 支持同时选中资产和Actor进行重命名

- 支持普通查找，区分大小写

- 支持正则表达式查找以及捕获组引用

- 支持添加前缀和后缀

- 支持编号

- 支持改变大小写（只对英文字母有效）：不做改变、替换的字母转为小写、替换的字母转为大写、全部字母转为小写、全部字母转为大写

---

### Notes:

Ensure names comply with Unreal Engine specifications. The plugin's input fields do not validate naming conventions, and invalid names will trigger Unreal Engine errors and cause renaming to fail.

For batch renaming, avoid selecting tens of thousands of items at once. Instead, split large batches into smaller groups.

The renaming logic uses Unreal Engine's native API. If renaming fails, verify that the items can be renamed manually without the plugin first.

#### 注意事项：

- 请确保名称符合虚幻引擎规范，插件中的输入框并不会检测你的命名是否规范，不规范的命名将在重命名时由虚幻引擎提示，同时会导致重命名失败。

- 批量重命名的数量最好不要成千上万，如果成千上万应该分批次进行，而不是一次性选中成千上万的待重命名条目

- 重命名底层实现调用的虚幻引擎提供的 API，如果重命名失败，还请确保不使用该插件的前提下也能对条目进行重命名操作
