export module zeytin.game.aliases;
export import zeytin.game.alias;

import zeytin.game.card;
import zeytin.game.transform;
import zeytin.game.collider;
import zeytin.game.card_renderer;

export {
	using CardAlias = Alias<CCard, CTransform, CCollider, CCardRenderer>;
}
