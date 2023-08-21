# TODO
I need to think of a better way of handling the whole getMinWidth/Height thing. Currently if there is a widget that has a dimension that depends on the other there will be some problems getting a result that will match the layout version. For example getting the min height of a text: if you have a text that can wrap then getting its min height will depend on the final width of the text which can't really be known.
Essentially what this min width/height is trying to do at the end of the day is calculate the layout size which could probably be done in a better way.
Maybe passing down a flag down the layout tree that tells the widget to calculate the size as if it was Size::Shrink. There would still be a problem left with widgets like column. Telling a column to shrink horizontally will need to to have the width of the biggest child and is not difficult, but what if the widgets inside want to expand? All the children would need to know the biggest size which would require you to do the layout twice. 

# Grid
A container that arranges its children in a grid pattern

## Arguments
- widget: Widget::Args
    - In the case where both `width` and `height` are `Size::Shrink` then the grid requires that either `columns` or `rows` have a constant count (not `minSize`)
- columns: Template
- rows: Template
- spacing: std::variant of `both: float` or `specific: std::pair<float, float>`
    - default: `0.0f`
- children: Children
    - If there are more children than the grid can fit, the extra children will be ignored

## Types
### Flex
std::initializer_list of std::variant of `fraction: float` or `fixed: float`

### Template
- Template
    - count: std::optional of int
    - count: std::variant of `count: int` or `minSize: float` or `flex: Flex`
    - 
