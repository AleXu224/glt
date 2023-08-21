# Row spec

## getMinWidth

In all cases getMinWidth should respect the provided maxSize and the sizeConstraints of the widget.

### fixed width
When the width is fixed the minimum width should be widget size + the margin.

### variable width (shrink or expand)
The maximum space allowed to take by the widgets will the difference between the maxSize and the spacing between the widgets.
In this case the minimum width should be the sum of minimum widths of all the widgets + the provided spacing.

In the case where a child widget or more are expanded then the remaining space after the other widgets have been given their minimum width will be equally divided among them.

## getMinHeight

In all cases getMinHeight should respect the provided maxSize and the sizeConstraints of the widget.

### fixed height
When the height is fixed the minimum height should be widget size + the margin.

### variable height (shrink or expand)
This is a made more complicated by the fact that some widgets have a different height depending on the width.
Because of this the minimum width for all widgets will also need to be computed.

We follow the same rules as getMinWidth but also keep track of the biggest minimum height of all the widgets.