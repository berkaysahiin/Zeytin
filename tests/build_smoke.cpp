import zeytin.common.guid;

int main() {
    const auto first = generate_stable_id("zeytin.build.smoke");
    const auto repeated = generate_stable_id("zeytin.build.smoke");
    const auto retried = generate_stable_id("zeytin.build.smoke", 1);

    if (first == 0 || first != repeated || first == retried) {
        return 1;
    }

    return 0;
}
