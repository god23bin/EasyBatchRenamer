// Copyright Devil Dev, 2025.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "RenamerDataTypes.h"

class EASYBATCHRENAMER_API SRenamerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRenamerWidget)
	{}
	SLATE_END_ARGS()

	SRenamerWidget();

	void Construct(const FArguments& InArgs);
	
	virtual ~SRenamerWidget() override;

protected:
	TSharedRef<SWidget> CreateMainContentLayout();

	TSharedRef<SWidget> CreateRenameOptionsArea();
	TSharedRef<SWidget> CreateNoSelectionMessageArea();
	TSharedRef<SWidget> CreateRenameListViewArea();
	
private:

	FString FindText;
	FString ReplaceText;
	bool bEnableRegexFind;
	FString RegexFindText;
	FString LastRegexFindText;
	TSharedPtr<FRegexPattern> CachedRegexPattern;
	FString PrefixText;
	FString SuffixText;
	bool bUseNumbering;
	int32 StartNumber;
	int32 PaddingDigits;
	constexpr static int32 DefaultStartNumber = 1;
	constexpr static int32 DefaultPaddingDigits = 3;
	ETextCaseChange CaseChangeOption;

	bool bAnyItemNeedsRename;
	
	TSharedPtr<SEditableTextBox> FindTextBoxWidget;
	TSharedPtr<SEditableTextBox> RegexFindTextBoxWidget;
	
	TArray<TSharedPtr<ETextCaseChange>> CaseChangeOptions; 
	
	TArray<TSharedPtr<FRenameItem>> RenameItems;

	TSharedPtr<SListView<TSharedPtr<FRenameItem>>> RenameListView;

	TSharedPtr<STextBlock> NoSelectionMessageText;

	TSharedRef<SWidget> GenerateCaseChangeComboBoxEntry(TSharedPtr<ETextCaseChange> InOption);

	/**
	 * @brief 重置所有规则
	 */
	void ResetAllRulesInternal();

	/**
	 * @brief 应用所有重命名规则
	 */
	void ApplyAllRenameRules(bool bNeedRequireRefreshListViewByCurrentFunction = false);
	
	/**
	 * @brief 生成 SListView 的每一行
	 * @param InItem FRenameItem 数据
	 * @param OwnerTable 列表视图
	 * @return Slate 行
	 */
	TSharedRef<ITableRow> HandleGenerateRow(TSharedPtr<FRenameItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/**
	 * @brief 当内容浏览器选中项改变时调用
	 * @param SelectedAssets 新选中的资产数据
	 * @param bFromUser 是否由用户交互引起的选择变化
	 */
	void OnContentBrowserSelectionChanged(const TArray<FAssetData>& SelectedAssets, bool bFromUser);

	/**
	 * @brief 当 Actor 选中项改变时调用
	 * @param InSelectedActor 最近被选择或取消选择的 Actor
	 */
	void OnActorSelectionChanged(UObject* InSelectedActor);

	/**
	 * @brief 聚合函数，用于更新 RenameItems 列表并刷新 UI
	 * 它将内容浏览器中的资产和大纲视图中的 Actor 合并
	 */
	void UpdateRenameListFromSelection();

	/**
	 * @brief 检查是否有选中的资产或 Actor
	 * @return 如果有选中项则返回 true
	 */
	bool HasSelection() const;

	/**
	 * @brief 检查是否可以执行重命名操作
	 * 判断是否有需要重命名的项且输入参数有效
	 * @return 如果可以重命名则返回 true
	 */
	bool CanApplyRename() const;

	/**
	 * @brief 处理点击“应用重命名”按钮的事件
	 * 执行实际的重命名操作并更新 UI
	 * @return 返回操作结果
	 */
	FReply OnApplyRenameButtonClicked();

	/**
	 * @brief 获取 FindText 输入框是否启用的状态
	 * 根据是否启用正则表达式查找来决定输入框的可用性
	 * @return 如果输入框可用则返回 true，否则返回 false
	 */
	bool GetFindTextBoxIsEnabled() const;

	/**
	 * @brief 获取 RegexFindText 输入框是否启用的状态
	 * 根据是否启用正则表达式查找来决定输入框的可用性
	 * @return 如果输入框可用则返回 true，否则返回 false
	 */
	bool GetRegexTextBoxIsEnabled() const;

	/**
	 * @brief 检查当前的重命名规则是否已被修改
	 * 用于判断用户是否更改了任何重命名规则（如 Find/Replace 文本、前缀、后缀等）
	 * @return 如果规则被修改则返回 true
	 */
	bool AreRulesModified() const;
	
	/**
	 * @brief 获取正则表达式启用复选框的当前状态
	 * 根据是否启用正则表达式查找返回相应的 ECheckBoxState 状态值
	 * @return ECheckBoxState::Checked 如果启用了正则表达式，否则返回 ECheckBoxState::Unchecked
	 */
	ECheckBoxState GetEnableRegexCheckBoxState() const;

	/**
	 * @brief 获取当前大小写改变选项的文本
	 * @return 大小写改变选项文本
	 */
	FText GetCurrentCaseChangeComboBoxText() const;

	/**
	 * @brief 获取第一列（原始名称）的文本
	 * @param InItem FRenameItem 数据
	 * @return 原始名称的文本
	 */
	FText GetOriginalNameText(TSharedPtr<FRenameItem> InItem) const;
	
	/**
	 * @brief 获取第二列（预览名称）的文本
	 * @param InItem FRenameItem 数据
	 * @return 预览名称的文本
	 */
	FText GetPreviewNameText(TSharedPtr<FRenameItem> InItem) const;
	
	/**
	 * @brief 获取第三列（类型）的文本
	 * @param InItem FRenameItem 数据
	 * @return 类型的文本
	 */
	FText GetItemTypeText(TSharedPtr<FRenameItem> InItem) const;
	
	/**
	 * @brief 获取第四列（手动名称）的文本
	 * @param InItem FRenameItem 数据
	 * @return 手动名称的文本
	 */
	FText GetManualNameText(TSharedPtr<FRenameItem> InItem) const;

	/**
	 * @brief 当 FindText 输入框文本改变时调用
	 * 更新当前的查找文本并应用重命名规则
	 * @param NewText 新的输入文本
	 */
	void OnFindTextChanged(const FText& NewText);

	/**
	 * @brief 当正则表达式启用复选框状态改变时调用
	 * 根据新的复选框状态启用或禁用正则表达式查找功能
	 * @param NewState 新的复选框状态
	 */
	void OnEnableRegexCheckBoxChanged(ECheckBoxState NewState);

	/**
	 * @brief 当 RegexFindText 输入框文本改变时调用
	 * 更新当前的正则表达式查找文本并应用重命名规则
	 * @param NewText 新的输入文本
	 */
	void OnRegexFindTextChanged(const FText& NewText);

	/**
	 * @brief 当 ReplaceText 输入框文本改变时调用
	 * 更新当前的替换文本并应用重命名规则
	 * @param NewText 新的输入文本
	 */
	void OnReplaceTextChanged(const FText& NewText);

	/**
	 * @brief 当 PrefixText 输入框文本改变时调用
	 * 更新当前的前缀文本并应用重命名规则
	 * @param NewText 新的输入文本
	 */
	void OnPrefixTextChanged(const FText& NewText);

	/**
	 * @brief 当 SuffixText 输入框文本改变时调用
	 * 更新当前的后缀文本并应用重命名规则
	 * @param NewText 新的输入文本
	 */
	void OnSuffixTextChanged(const FText& NewText);

	/**
	 * @brief 当 UseNumbering 复选框状态改变时调用
	 * 根据新的复选框状态启用或禁用编号功能
	 * @param NewState 新的复选框状态
	 */
	void OnUseNumberingChanged(ECheckBoxState NewState);

	/**
	 * @brief 当 StartNumber 输入框数值改变时调用
	 * 更新起始编号并刷新预览名称
	 * @param NewValue 新的数值
	 */
	void OnStartNumberChanged(int32 NewValue);

	/**
	 * @brief 当 PaddingDigits 输入框数值改变时调用
	 * 更新填充位数并刷新预览名称
	 * @param NewValue 新的数值
	 */
	void OnPaddingDigitsChanged(int32 NewValue);

	/**
	 * @brief 当大小写改变选项下拉框选择改变时调用
	 * 更新当前的大小写转换选项并刷新预览名称
	 * @param NewSelection 新的选择项
	 * @param SelectInfo 选择信息
	 */
	void OnCaseChangeOptionChanged(TSharedPtr<ETextCaseChange> NewSelection, ESelectInfo::Type SelectInfo);

	/**
	 * @brief 点击撤销上次重命名按钮时调用
	 * 恢复所有重命名项到重命名之前的状态
	 * @return 返回操作结果
	 */
	FReply OnUndoLastRenameClicked();

	/**
	 * @brief 判断是否可以撤销上次重命名
	 * 检查是否有可撤销的操作存在
	 * @return 如果可以撤销则返回 true
	 */
	bool CanUndoLastRename() const;

	/**
	 * @brief 点击重置所有规则按钮时调用
	 * 将所有重命名规则参数恢复为默认值，并刷新 UI
	 * @return 返回操作结果
	 */
	FReply OnResetAllRulesClicked();

	/**
	 * @brief 当手动名称输入框文本改变时调用
	 * 实时更新指定项的手动名称并在 UI 中刷新预览
	 * @param NewText 新的输入文本
	 * @param InItem 被修改的 FRenameItem 对象
	 */
	void OnManualNameTextChanged(const FText& NewText, TSharedPtr<FRenameItem> InItem);

	/**
	 * @brief 当手动名称输入框文本提交时调用
	 * 在用户完成编辑并提交后更新指定项的手动名称
	 * @param NewText 提交的新文本
	 * @param CommitType 提交类型（如按下回车、失去焦点等）
	 * @param InItem 被修改的 FRenameItem 对象
	 */
	void OnManualNameTextCommitted(const FText& NewText, ETextCommit::Type CommitType, TSharedPtr<FRenameItem> InItem);



};
