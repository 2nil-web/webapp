var ttty = function(f) {
    "use strict";
    const c = (t, n, e) => {
            const o = document.createElement(t);
            return n && (o.className = n), e && (o.innerHTML = e), o
        },
        N = (t, n = !1) => {
            t.input.readOnly = !n, t.inputContainer.style.opacity = n ? "" : "0"
        },
        C = (t, n) => {
            t.className = "terminal";
            const e = c("div", "terminal-container"),
                o = c("div", "terminal-type"),
                s = c("span", void 0, n),
                r = c("input");
            return r.setAttribute("type", "text"), o.append(s), o.append(r), t.append(e), t.append(o), t.addEventListener("click", () => r.focus()), {
                commandContainer: e,
                input: r,
                inputContainer: o
            }
        },
        T = (t, n, e, o, s, r) => {
            const a = c("p", void 0, n ? r.prompt : t);
            if (n) {
                const i = c("span", "terminal-command", t);
                a.append(i)
            }
            o.append(a), e && s.scrollIntoView()
        };
    var p = (t => (t.ON_COMMAND = "onCommand", t.ON_COMMAND_NOT_FOUND = "onCommand404", t.ON_PROCESS_START = "onProcessStart", t.ON_PROCESS_END = "onProcessStop", t.ON_PROCESS_INTERRUPT = "onProcessInterrupt", t.ON_INIT = "onInit", t))(p || {});
    const m = (t, n, e) => {
            const o = new CustomEvent(t, {
                detail: e
            });
            n.dispatchEvent(o)
        },
        v = (t, n) => {
            const {
                print: e
            } = n, [o, ...s] = t.split(" "), r = s.join(" ").match(/('[^']+'|"[^"]+"|[^\s'"]+)/g), a = r === null ? [] : r.map(u => u.replace(/(^['"]|['"]$)/g, "")), i = n.settings.commands[o];
            i ? i.argDescriptions && i.argDescriptions.length > 0 && a.length === 0 ? e(`Usage: ${o} ${i.argDescriptions.map(u=>`[${u}]`).join(" ")}`) : (i.func(n, ...a), m(p.ON_COMMAND, n.settings.host, t)) : (e(`<span class="terminal-error">command not found: ${o}</span>`), m(p.ON_COMMAND_NOT_FOUND, n.settings.host, t))
        },
        _ = t => {
            const {
                settings: {
                    host: n
                }
            } = t;
            N(t), t.isProcessRunning = !0, m(p.ON_PROCESS_START, n)
        },
        y = t => {
            const {
                input: n,
                settings: {
                    host: e
                }
            } = t;
            N(t, !0), t.isProcessRunning = !1, n.focus(), m(p.ON_PROCESS_END, e)
        },
        R = (t, n, e, o) => new Promise(s => {
            _(e);
            const r = c("p", void 0, o ? e.settings.prompt : ""),
                a = c("span", "terminal-command", "");
            o && r.append(a), e.commandContainer.append(r);
            let i = 0;
            const u = o ? a : r,
                g = () => {
                    i < t.length && e.isProcessRunning ? (u.innerHTML += t.charAt(i), i++, setTimeout(g, n)) : (y(e), s(i === t.length))
                };
            setTimeout(g, n)
        }),
        I = t => {
            const {
                input: {
                    value: n
                },
                history: e,
                settings: {
                    historyLength: o
                }
            } = t;
            if (n !== e[0]) {
                if (e.length >= o) {
                    t.history = [n, ...e.slice(0, -1)];
                    return
                }
                t.history = [n, ...e], t.lastHistoryIndex = 0
            }
        },
        P = (t, n) => {
            const {
                history: e,
                lastHistoryIndex: o
            } = t, s = e.length - 1;
            let r;
            n && o === 0 || !n && o === e.length || s < 0 || (n ? (r = o - 1, t.input.value = r - 1 >= 0 ? e[r - 1] : "") : (r = o + 1, t.input.value = e[o]), t.lastHistoryIndex = r)
        },
        b = (t, n) => {
            const {
                input: e,
                print: o
            } = n;
            t.addEventListener("keyup", ({
                key: s,
                ctrlKey: r
            }) => {
                if (r && s === "c" && n.isProcessRunning) o("^C"), y(n), m(p.ON_PROCESS_INTERRUPT, t);
                else {
                    if (n.isProcessRunning) return;
                    if (s === "Enter") {
                        n.lastHistoryIndex = 0, e.value.length > 0 ? (o(e.value, !0), I(n), v(e.value, n)) : o(" ", !0), e.value = "";
                        return
                    }
                    if (s === "ArrowUp") {
                        P(n);
                        return
                    }
                    s === "ArrowDown" && P(n, !0)
                }
            })
        },
        E = t => ({
            name: "help",
            description: "shows a full list of all available commands",
            func: ({
                print: n
            }) => {
                for (const e in t.settings.commands) Object.hasOwnProperty.call(t.settings.commands, e) && n(t.settings.commands[e].name + " - " + t.settings.commands[e].description)
            }
        }),
        A = '.terminal{overflow-y:auto;overflow-x:hidden;background-color:var(--terminal-bg-color, black);padding:15px;box-sizing:border-box}.terminal *{color:var(--terminal-fg-color, white);font-family:var(--terminal-font, "Courier New", monospace)}.terminal-type{width:100%;display:flex}.terminal-type>span{padding-right:7pt}.terminal-type>input{flex-grow:2;background:transparent;border:0;font-size:inherit;padding:0}.terminal-type>input:focus{border:none;outline:none}.terminal-container>p{margin:0}span.terminal-command{color:var(--terminal-accent-color, #ffff7d)}span.terminal-error{color:var(--terminal-error-color, #cc1010)}',
        M = () => {
            if (document.head.querySelectorAll('link[data-type="terminal"]').length === 0) {
                const t = c("style", void 0, A);
                t.setAttribute("data-type", "terminal"), t.setAttribute("type", "text/css"), document.head.append(t)
            }
        },
        w = (t, n, e) => {
            e.prompt = t;
            const o = n.querySelector("span");
            o.innerHTML = t
        },
        H = ({
            host: t,
            welcomeMessage: n,
            prompt: e = "$: ",
            historyLength: o = 50,
            history: s = [],
            enableHelp: r = !0,
            commands: a
        }) => {
            const i = {
                host: t,
                welcomeMessage: n,
                prompt: e,
                historyLength: o,
                enableHelp: r,
                commands: a
            };
            M();
            const {
                commandContainer: u,
                input: g,
                inputContainer: S
            } = C(t, e), l = {
                history: s,
                lastHistoryIndex: 0,
                isProcessRunning: !1,
                settings: i,
                commandContainer: u,
                inputContainer: S,
                input: g,
                print: (d, h = !1, O = !0) => T(d, h, O, u, g, i),
                run: d => v(d, l),
                start: () => _(l),
                stop: () => y(l),
                type: (d, h = 60, O) => R(d, h, l, O),
                setPrompt: d => w(d, S, i)
            };
            return r && (l.settings.commands.help = E(l)), b(t, l), m(p.ON_INIT, t), n && l.print(n), l
        };
    return f.TerminalEvent = p, f.initTerminal = H, Object.defineProperties(f, {
        __esModule: {
            value: !0
        },
        [Symbol.toStringTag]: {
            value: "Module"
        }
    }), f
}({});