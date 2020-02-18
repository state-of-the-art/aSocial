/**
 * Markdown library wrapper
 * Usage:
 *  import "js/markdown.lib.js" as MD
 *  Text {
 *      text: MD.makeHtml("Markdown *rules*!")
 *  }
**/
.pragma library
.import "lib/Markdown.Converter.js" as M

var _md = new M.Markdown.Converter()

function makeHtml(text) {
    return _md.makeHtml(text)
}
