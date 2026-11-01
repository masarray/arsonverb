/* Tiny progressive enhancement: OS selector, keyboard support, URL deeplinks. */
(() => {
  const tabs = [...document.querySelectorAll('[data-platform]')];
  const panels = [...document.querySelectorAll('[data-panel]')];
  if (tabs.length !== 3 || panels.length !== 3) return;

  function activate(platform, moveFocus = false) {
    if (!tabs.some(tab => tab.dataset.platform === platform)) return;
    for (const tab of tabs) {
      const selected = tab.dataset.platform === platform;
      tab.classList.toggle('is-active', selected);
      tab.setAttribute('aria-selected', String(selected));
      tab.tabIndex = selected ? 0 : -1;
      if (selected && moveFocus) tab.focus();
    }
    for (const panel of panels) {
      const shown = panel.dataset.panel === platform;
      panel.classList.toggle('is-active', shown);
      panel.hidden = !shown;
    }
  }

  const query = new URLSearchParams(window.location.search);
  const initial = (query.get('os') || '').toLowerCase();
  activate(['windows','macos','linux'].includes(initial) ? initial : 'windows');

  tabs.forEach((tab, index) => {
    tab.addEventListener('click', () => activate(tab.dataset.platform));
    tab.addEventListener('keydown', event => {
      if (!['ArrowLeft','ArrowRight','Home','End'].includes(event.key)) return;
      event.preventDefault();
      let next = index;
      if (event.key === 'Home') next = 0;
      if (event.key === 'End') next = tabs.length - 1;
      if (event.key === 'ArrowLeft') next = (index + tabs.length - 1) % tabs.length;
      if (event.key === 'ArrowRight') next = (index + 1) % tabs.length;
      activate(tabs[next].dataset.platform, true);
    });
  });
})();