# Bench Log

Append-only record of Phase 0 measurements. Newest entry first. Every session
that produces a number belongs here, especially the ones that disagree with
the estimates in [DESIGN_V2.md](DESIGN_V2.md).

Workflow rules for this file: [WORKFLOW.md](WORKFLOW.md) section 2.

## Entry template

```markdown
## YYYY-MM-DD: short title

**Setup:** hardware, firmware commit, print settings, anything needed to reproduce
**Measured:**
- metric: value (target: x, from DESIGN_V2 sec N)
**Verdict:** pass / fail / inconclusive, and why
**Changes:** params edited, docs corrected, TODO items checked
```

---

(no entries yet; the first one should be the TMAG5273 swing/noise bench)
