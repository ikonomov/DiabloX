/**
 * @file help.cpp
 *
 * Implementation of the in-game help text.
 */
#include <string>
#include <vector>

#include "DiabloUI/ui_flags.hpp"
#include "engine/render/clx_render.hpp"
#include "engine/render/text_render.hpp"
#include "init.h"
#include "minitext.h"
#include "qol/chatlog.h"
#include "stores.h"
#include "utils/language.h"
#include "utils/stdcompat/string_view.hpp"

namespace devilution {

bool HelpFlag;

namespace {

unsigned int SkipLines;

const char *const HelpText[] = {
	N_("$Shrines:"),
	N_("Abandoned: +2 dexterity"),
	N_("Creepy: +2 strength"),
	N_("Cryptic: casts Nova and restores mana"),
	N_("Divine: restores health and mana, gives two potions"),
	N_("Eerie: +2 magic"),
	N_("Eldritch: all potions become rejuvenation potions"),
	N_("Enchanted: -1 slvl to one spell, +1 to all other known spells"),
	N_("Fascinating: lose 10% of base magic, +2 slvl to Firebolt"),
	N_("Glimmering: identifies all items"),
	N_("Gloomy: +2 AC to all armor, -1 max damage to all weapons"),
	N_("Hidden: -10 durability to one equipped item, +10 to all others"),
	N_("Holy: casts Phasing"),
	N_("Magical: casts Mana Shield"),
	N_("Mysterious: +5 to one attribute, -1 to all others"),
	N_("Ornate: lose 10% of base magic, +2 slvl to Holy Bolt"),
	N_("Quiet: +2 vitality"),
	N_("Religious: restores durability of all items"),
	N_("Sacred: lose 10% of base magic, +2 slvl to Charged Bolt"),
	N_("Secluded: reveals map of current level"),
	N_("Spiritual: fills inventory with gold"),
	N_("Spooky: restores life and mana of other players"),
	N_("Stone: restores charges of all staves"),
	N_("Tainted: other players get +1 to one attribute, -1 to all others"),
	N_("Thaumaturgic: refills chests on current level"),
	N_("Weird: +1 max damage to all weapons"),
	"",
	N_("$Other Objects:"),
	N_("Blood Fountain: restores life"),
	N_("Cauldron: random shrine effect"),
	N_("Fountain of Tears: +1 to one attribute, -1 to another"),
	N_("Goat Shrine: random shrine effect"),
	N_("Murky Pool: casts Infravision"),
	N_("Purifying Spring: restores mana"),
};

std::vector<std::string> HelpTextLines;

constexpr int PaddingTop = 32;
constexpr int PaddingLeft = 32;

constexpr int PanelHeight = 297;
constexpr int ContentTextWidth = 565;

int LineHeight()
{
	return IsSmallFontTall() ? 18 : 14;
}

int BlankLineHeight()
{
	return 12;
}

int DividerLineMarginY()
{
	return BlankLineHeight() / 2;
}

int HeaderHeight()
{
	return PaddingTop + LineHeight() + 2 * BlankLineHeight() + DividerLineMarginY();
}

int ContentPaddingY()
{
	return BlankLineHeight();
}

int ContentsTextHeight()
{
	return PanelHeight - HeaderHeight() - DividerLineMarginY() - 2 * ContentPaddingY() - BlankLineHeight();
}

int NumVisibleLines()
{
	return (ContentsTextHeight() - 1) / LineHeight() + 1; // Ceil
}

void DrawHelpSlider(const Surface &out)
{
	const Point uiPosition = GetUIRectangle().position;
	const int sliderXPos = ContentTextWidth + uiPosition.x + 36;
	int sliderStart = uiPosition.y + HeaderHeight() + LineHeight() + 3;
	int sliderEnd = uiPosition.y + PaddingTop + PanelHeight - 12;
	ClxDraw(out, { sliderXPos, sliderStart }, (*pSTextSlidCels)[11]);
	sliderStart += 12;
	int sliderCurrent = sliderStart;
	for (; sliderCurrent < sliderEnd; sliderCurrent += 12) {
		ClxDraw(out, { sliderXPos, sliderCurrent }, (*pSTextSlidCels)[13]);
	}
	ClxDraw(out, { sliderXPos, sliderCurrent }, (*pSTextSlidCels)[10]);
	// Subtract visible lines from the total number of lines to get the actual
	// scroll range
	const int scrollRange = static_cast<int>(HelpTextLines.size()) - NumVisibleLines();
	// Subtract the size of the arrow buttons to get the length of the interior
	// part of the slider
	const int sliderLength = sliderCurrent - 12 - sliderStart;
	ClxDraw(out, { sliderXPos, sliderStart + ((static_cast<int>(SkipLines) * sliderLength) / scrollRange) }, (*pSTextSlidCels)[12]);
}

} // namespace

void InitHelp()
{
	static bool Initialized = false;
	if (Initialized)
		return;

	HelpFlag = false;

	for (const auto *text : HelpText) {
		const std::string paragraph = WordWrapString(_(text), ContentTextWidth);

		size_t previous = 0;
		while (true) {
			size_t next = paragraph.find('\n', previous);
			HelpTextLines.emplace_back(paragraph.substr(previous, next - previous));
			if (next == std::string::npos)
				break;
			previous = next + 1;
		}
	}

	Initialized = true;
}

void DrawHelp(const Surface &out)
{
	DrawSTextHelp();
	DrawQTextBack(out);

	const int lineHeight = LineHeight();
	const int blankLineHeight = BlankLineHeight();

	string_view title;
	if (gbIsHellfire)
		title = gbIsSpawn ? _("Shareware Hellfire Help") : _("Hellfire Help");
	else
		title = gbIsSpawn ? _("Shareware Diablo Help") : _("Shrine List");

	const Point uiPosition = GetUIRectangle().position;
	const int sx = uiPosition.x + PaddingLeft;
	const int sy = uiPosition.y;

	DrawString(out, title,
	    { { sx, sy + PaddingTop + blankLineHeight }, { ContentTextWidth, lineHeight } },
	    UiFlags::ColorWhitegold | UiFlags::AlignCenter);

	const int titleBottom = sy + HeaderHeight();
	DrawSLine(out, titleBottom);

	const int numLines = NumVisibleLines();
	const int contentY = titleBottom + DividerLineMarginY() + ContentPaddingY();
	for (int i = 0; i < numLines; i++) {
		const string_view line = HelpTextLines[i + SkipLines];
		if (line.empty()) {
			continue;
		}

		int offset = 0;
		UiFlags style = UiFlags::ColorWhite;
		if (line[0] == '$') {
			offset = 1;
			style = UiFlags::ColorBlue;
		}

		DrawString(out, line.substr(offset), { { sx, contentY + i * lineHeight }, { ContentTextWidth, lineHeight } }, style, /*spacing=*/1, lineHeight);
	}

	DrawString(out, _("Press ESC to end or the arrow keys to scroll."),
	    { { sx, contentY + ContentsTextHeight() + ContentPaddingY() + blankLineHeight }, { ContentTextWidth, lineHeight } },
	    UiFlags::ColorWhitegold | UiFlags::AlignCenter);

	DrawHelpSlider(out);
}

void DisplayHelp()
{
	SkipLines = 0;
	HelpFlag = true;
	ChatLogFlag = false;
}

void HelpScrollUp()
{
	if (SkipLines > 0)
		SkipLines--;
}

void HelpScrollDown()
{
	if (SkipLines + NumVisibleLines() < HelpTextLines.size())
		SkipLines++;
}

} // namespace devilution
