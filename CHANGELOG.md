# Next

* Add support to reading images from a `bytes` buffer, e.g.
  ```python 
  with open("chicago.jpg", "rb") as f:
      img = accimage.Image(f.read())
  ```

# v0.1.1

* Bug fix: Horizontal crops prior to v0.1.1 didn't work.
* Add pytest tests.py
* Add travis building support

# v0.1.0

Initial release
