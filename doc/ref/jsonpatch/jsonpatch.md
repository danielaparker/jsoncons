### jsonpatch extension

The jsonpatch extension implements the IETF standard [JavaScript Object Notation (JSON) Patch](https://tools.ietf.org/html/rfc6902)

<table border="0">
  <tr>
    <td><a href="apply_patch.md">apply_patch</a></td>
    <td>Apply JSON Patch operations to a JSON document.</td> 
  </tr>
  <tr>
    <td><a href="from_diff.md">from_diff</a></td>
    <td>Create a JSON patch from a diff of two JSON documents.</td> 
  </tr>
</table>

The JSON Patch IETF standard requires that the JSON Patch method is atomic, so that if any JSON Patch operation results in an error, the target document is unchanged.
The patch function implements this requirement by generating the inverse commands and building an undo stack, which is executed if any part of the patch fails.


