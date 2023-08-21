# Layout
Represents the way a Widget will calculate its size (without position) based on its state and parent constraints.

## Parent constraints
A parent will provide a maximum size to its children. This constraint is not always respected, as a child may not be able to become small enough to fit the parent constraints. In this case, the child will overflow the parent.

## Width and Height
### Fixed
A Widget with a fixed width or height will always have the same size, regardless of the constraints given by the parent or the overflow of its children.

### Expand
A Widget that expands will try to take as much space as possible, as big as the parent constraints allow it to. An expanded widget will overflow its parent if it is not able to become small enough to contain its children.

### Shrink
A Widget that shrinks will try to take as little space as possible, as small as the content allows it to. A shrunk widget will overflow its parent if it is not able to become small enough to contain its children.

## Size Constraints
Not to be confused with parent constraints, size constraints are provided by the user and are to be strictly respected by the widget even if it means overflowing its parent or not being able to contain its children.