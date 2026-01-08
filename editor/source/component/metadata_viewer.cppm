module;

export module zeytin.metadata.viewer;

export class MetadataViewer {
public:
    MetadataViewer();
    ~MetadataViewer();
    
    void render();
    
private:
    void render_variant_list();
    void render_variant_details();
    
    int m_selected_variant_index = -1;
};
