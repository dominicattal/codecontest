var cur_shown = null;
function showProblem(letter) {
if (cur_shown) cur_shown.setAttribute("hidden", "");
ele = document.getElementById(`problem-${letter}`);
ele.removeAttribute("hidden");
cur_shown = ele;
}
showProblem('A');
