import invariant from 'invariant';


export class RefMap {
  constructor(core) {
    this._map = new Map();
    this._core = core;
  }

  getOrCreate(name, type, props, children) {
    if (!this._map.has(name)) {
      let ref = this._core.createRef(type, props, children);
      this._map.set(name, ref);
    }

    return this._map.get(name)[0];
  }

  update(name, props) {
    invariant(this._map.has(name), "Oops, trying to update a ref that doesn't exist");

    let [node, setter] = this._map.get(name);
    setter(props);
  }
}
