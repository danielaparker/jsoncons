### jsonpatch extension

The jsonpatch extension implements the IETF standard [JavaScript Object Notation (JSON) Patch](https://tools.ietf.org/html/rfc6902)

[patch](patch.md)  

<table border="0">
  <tr>
    <td><a href="patch.md">patch</a></td>
    <td>Apply JSON Patch operations to a JSON document.</td> 
  </tr>
</table>

The JSON Patch IETF standard requires that the JSON PATCH method is atomic, so that if any JSON patch operation results in an error, the target document is unchanged.  
The patch function implements this requirement by generating the inverse commands and building an undo stack, which is executed if any part of the patch fails.


